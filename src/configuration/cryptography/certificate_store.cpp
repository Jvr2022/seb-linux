#include "certificate_store.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QMetaType>
#include <QSslKey>

#include <openssl/asn1.h>
#include <openssl/x509.h>

#if SEB_HAS_LIBSECRET
#pragma push_macro("signals")
#undef signals
#include <libsecret/secret.h>
#pragma pop_macro("signals")
#endif

namespace seb::configuration::cryptography {
namespace {

constexpr int IdentityCertificateType = 1;

#if SEB_HAS_LIBSECRET
constexpr char IdentitySchemaName[] = "org.safeexambrowser.seb.identity";
constexpr char IdentityHashAttribute[] = "public-key-sha1";
constexpr char IdentitySecretContentType[] = "application/x-pkcs12";

const SecretSchema IdentitySchema = {
    IdentitySchemaName,
    SECRET_SCHEMA_NONE,
    {
        {IdentityHashAttribute, SECRET_SCHEMA_ATTRIBUTE_STRING},
        {nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING},
    },
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
#endif

QByteArray publicKeyHash(const QSslCertificate &certificate)
{
    const QByteArray encodedCertificate = certificate.toDer();
    if (encodedCertificate.isEmpty()) {
        return {};
    }

    const unsigned char *cursor = reinterpret_cast<const unsigned char *>(encodedCertificate.constData());
    X509 *x509 = d2i_X509(nullptr, &cursor, encodedCertificate.size());
    if (x509 == nullptr) {
        return {};
    }

    const ASN1_BIT_STRING *keyValue = X509_get0_pubkey_bitstr(x509);
    if (keyValue == nullptr) {
        X509_free(x509);
        return {};
    }

    const auto *bytes = ASN1_STRING_get0_data(reinterpret_cast<const ASN1_STRING *>(keyValue));
    const int length = ASN1_STRING_length(reinterpret_cast<const ASN1_STRING *>(keyValue));
    if (bytes == nullptr || length <= 0) {
        X509_free(x509);
        return {};
    }

    const QByteArray rawKeyValue(reinterpret_cast<const char *>(bytes), length);
    X509_free(x509);

    return QCryptographicHash::hash(rawKeyValue, QCryptographicHash::Sha1);
}

QByteArray certificateDataFrom(const QVariantMap &data)
{
    const QByteArray certificateData = data.value(QStringLiteral("certificateData")).toByteArray();
    if (!certificateData.isEmpty()) {
        return certificateData;
    }

    const QString base64 = data.value(QStringLiteral("certificateDataBase64")).toString();
    if (base64.isEmpty()) {
        return {};
    }

    const auto decoded = QByteArray::fromBase64Encoding(
        base64.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
    return decoded.decodingStatus == QByteArray::Base64DecodingStatus::Ok
        ? decoded.decoded
        : QByteArray{};
}

bool importIdentity(const QByteArray &data, QSslCertificate &certificate, QSslKey &privateKey)
{
    certificate = QSslCertificate();
    privateKey = QSslKey();

    QBuffer buffer;
    buffer.setData(data);
    if (!buffer.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray sebIdentityPassword = QString::fromUtf8(
        "Di𝈭l𝈖Ch𝈒ah𝉇t𝈁a𝉈Hai1972").toUtf8();
    if (!QSslCertificate::importPkcs12(
            &buffer, &privateKey, &certificate, nullptr, sebIdentityPassword)) {
        return false;
    }

    return !certificate.isNull() && !privateKey.isNull();
}

}  // namespace

CertificateStore::CertificateStore()
{
    loadPersistedIdentities();
}

bool CertificateStore::tryGetCertificateWith(const QByteArray &keyHash, QSslCertificate &certificate) const
{
    const auto it = certificates_.find(keyHash);
    if (it == certificates_.end()) {
        return false;
    }
    certificate = it.value();
    return true;
}

bool CertificateStore::tryGetPrivateKeyFor(const QSslCertificate &certificate, QSslKey &key) const
{
    const QByteArray keyHash = publicKeyHash(certificate);
    if (keyHash.isEmpty()) {
        return false;
    }

    const auto it = privateKeys_.find(keyHash);
    if (it == privateKeys_.end()) {
        return false;
    }

    key = it.value();
    return true;
}

void CertificateStore::loadPersistedIdentities()
{
#if SEB_HAS_LIBSECRET
    GError *error = nullptr;
    GList *items = secret_password_search_sync(
        &IdentitySchema,
        static_cast<SecretSearchFlags>(
            SECRET_SEARCH_ALL | SECRET_SEARCH_LOAD_SECRETS | SECRET_SEARCH_UNLOCK),
        nullptr,
        &error,
        nullptr);
    if (error != nullptr) {
        g_error_free(error);
    }

    for (GList *entry = items; entry != nullptr; entry = entry->next) {
        auto *item = SECRET_ITEM(entry->data);
        GHashTable *attributes = secret_item_get_attributes(item);
        const gchar *hashText = attributes == nullptr
            ? nullptr
            : static_cast<const gchar *>(g_hash_table_lookup(
                  attributes, IdentityHashAttribute));
        const SecretValue *secret = secret_item_get_secret(item);
        gsize secretLength = 0;
        const gchar *secretData = secret == nullptr
            ? nullptr
            : secret_value_get(const_cast<SecretValue *>(secret), &secretLength);

        const QByteArray data = secretData == nullptr
            ? QByteArray{}
            : QByteArray(secretData, static_cast<qsizetype>(secretLength));
        QSslCertificate certificate;
        QSslKey privateKey;
        if (data.isEmpty() || !importIdentity(data, certificate, privateKey)) {
            if (attributes != nullptr) {
                g_hash_table_unref(attributes);
            }
            continue;
        }

        const QByteArray keyHash = publicKeyHash(certificate);
        if (keyHash.isEmpty() || hashText == nullptr
            || QByteArray(hashText) != keyHash.toHex()) {
            if (attributes != nullptr) {
                g_hash_table_unref(attributes);
            }
            continue;
        }

        certificates_.insert(keyHash, certificate);
        privateKeys_.insert(keyHash, privateKey);
        if (attributes != nullptr) {
            g_hash_table_unref(attributes);
        }
    }

    g_list_free_full(items, g_object_unref);
#endif
}

bool CertificateStore::persistIdentity(const QByteArray &keyHash, const QByteArray &data) const
{
#if SEB_HAS_LIBSECRET
    if (keyHash.isEmpty() || data.isEmpty()) {
        return false;
    }

    const QByteArray hashText = keyHash.toHex();
    const QByteArray label = QByteArrayLiteral("Safe Exam Browser identity ") + hashText;
    SecretValue *secret = secret_value_new(
        data.constData(), data.size(), IdentitySecretContentType);
    if (secret == nullptr) {
        return false;
    }

    GError *error = nullptr;
    const gboolean stored = secret_password_store_binary_sync(
        &IdentitySchema,
        nullptr,
        label.constData(),
        secret,
        nullptr,
        &error,
        IdentityHashAttribute,
        hashText.constData(),
        nullptr);
    secret_value_unref(secret);
    if (error != nullptr) {
        g_error_free(error);
    }
    return stored != FALSE;
#else
    Q_UNUSED(keyHash)
    Q_UNUSED(data)
    return false;
#endif
}

void CertificateStore::extractAndImportIdentities(QVariantMap &data)
{
    const QString key = QStringLiteral("embeddedCertificates");
    const QVariant embeddedValue = data.value(key);
    if (!embeddedValue.isValid() || embeddedValue.metaType().id() != QMetaType::QVariantList) {
        return;
    }

    const QVariantList embeddedCertificates = embeddedValue.toList();
    QVariantList retainedCertificates;
    retainedCertificates.reserve(embeddedCertificates.size());

    for (const QVariant &value : embeddedCertificates) {
        const QVariantMap embeddedCertificate = value.toMap();
        bool typeIsValid = false;
        const int type = embeddedCertificate.value(QStringLiteral("type")).toInt(&typeIsValid);
        if (!typeIsValid || type != IdentityCertificateType) {
            retainedCertificates.append(value);
            continue;
        }

        const QByteArray certificateData = certificateDataFrom(embeddedCertificate);
        if (certificateData.isEmpty()) {
            retainedCertificates.append(value);
            continue;
        }

        QSslCertificate certificate;
        QSslKey privateKey;
        if (!importIdentity(certificateData, certificate, privateKey)) {
            retainedCertificates.append(value);
            continue;
        }

        const QByteArray keyHash = publicKeyHash(certificate);
        if (keyHash.isEmpty()) {
            retainedCertificates.append(value);
            continue;
        }

        certificates_.insert(keyHash, certificate);
        if (!privateKey.isNull()) {
            privateKeys_.insert(keyHash, privateKey);
        }
        if (!persistIdentity(keyHash, certificateData)) {
            retainedCertificates.append(value);
        }
    }

    data.insert(key, retainedCertificates);
}

}  // namespace seb::configuration::cryptography
