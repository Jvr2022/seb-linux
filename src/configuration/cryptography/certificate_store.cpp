#include "certificate_store.h"

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

void CertificateStore::extractAndImportIdentities(const QVariantMap &data)
{
    const QByteArray pem = data.value(QStringLiteral("embeddedCertificate")).toByteArray();
    if (pem.isEmpty()) {
        return;
    }

    const QSslCertificate certificate(pem);
    if (!certificate.isNull()) {
        certificates_.insert(certificate.digest(), certificate);
    }
}

}  // namespace seb::configuration::cryptography
