#include "certificate_store.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QMetaType>
#include <QSslKey>

#include <openssl/asn1.h>
#include <openssl/x509.h>

namespace seb::configuration::cryptography {
namespace {

constexpr int IdentityCertificateType = 1;

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

    const ASN1_BIT_STRING *encodedKey = X509_get0_pubkey_bitstr(x509);
    QByteArray encodedValue;
    if (encodedKey != nullptr) {
        const int length = i2d_ASN1_BIT_STRING(encodedKey, nullptr);
        if (length > 0) {
            encodedValue.resize(length);
            unsigned char *output = reinterpret_cast<unsigned char *>(encodedValue.data());
            if (i2d_ASN1_BIT_STRING(encodedKey, &output) != length) {
                encodedValue.clear();
            }
        }
    }
    X509_free(x509);

    return encodedValue.isEmpty()
        ? QByteArray{}
        : QCryptographicHash::hash(encodedValue, QCryptographicHash::Sha1);
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
    QBuffer buffer;
    buffer.setData(data);
    if (buffer.open(QIODevice::ReadOnly)) {
        const QByteArray sebIdentityPassword = QString::fromUtf8(
            "Di𝈭l𝈖Ch𝈒ah𝉇t𝈁a𝉈Hai1972").toUtf8();
        if (QSslCertificate::importPkcs12(
                &buffer, &privateKey, &certificate, nullptr, sebIdentityPassword)) {
            return true;
        }
        buffer.seek(0);
        if (QSslCertificate::importPkcs12(
                &buffer, &privateKey, &certificate, nullptr, QByteArray{})) {
            return true;
        }
    }

    certificate = QSslCertificate(data, QSsl::Der);
    if (certificate.isNull()) {
        certificate = QSslCertificate(data, QSsl::Pem);
    }
    return !certificate.isNull();
}

}  // namespace

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
    for (const auto &pair : identities_) {
        if (pair.first == certificate) {
            key = pair.second;
            return true;
        }
    }
    return false;
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
            identities_.append({certificate, privateKey});
        }
    }

    data.insert(key, retainedCertificates);
}

}  // namespace seb::configuration::cryptography
