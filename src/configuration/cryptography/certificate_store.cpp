#include "certificate_store.h"

#include <QSslKey>
#include <QDebug>

namespace seb::configuration::cryptography {

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

void CertificateStore::extractAndImportIdentities(const QVariantMap &data)
{
    // SEB usually stores identities in a list of dictionaries
    const QVariantList identities = data.value(QStringLiteral("identities")).toList();
    for (const QVariant &v : identities) {
        const QVariantMap m = v.toMap();
        const QByteArray certData = m.value(QStringLiteral("certificate")).toByteArray();
        const QByteArray keyData = m.value(QStringLiteral("privateKey")).toByteArray();

        if (certData.isEmpty()) continue;

        QSslCertificate cert(certData);
        if (cert.isNull()) {
            // Try base64
            cert = QSslCertificate(QByteArray::fromBase64(certData));
        }

        if (!cert.isNull()) {
            certificates_.insert(cert.digest(), cert);
            
            if (!keyData.isEmpty()) {
                QSslKey key(keyData, QSsl::Rsa); // Assuming RSA for SEB
                if (key.isNull()) {
                    key = QSslKey(QByteArray::fromBase64(keyData), QSsl::Rsa);
                }
                if (!key.isNull()) {
                    identities_.append({cert, key});
                }
            }
        }
    }

    // Also check for legacy 'embeddedCertificate' for backward compatibility
    const QByteArray pem = data.value(QStringLiteral("embeddedCertificate")).toByteArray();
    if (!pem.isEmpty()) {
        QSslCertificate certificate(pem);
        if (!certificate.isNull()) {
            certificates_.insert(certificate.digest(), certificate);
        }
    }
}

}  // namespace seb::configuration::cryptography
