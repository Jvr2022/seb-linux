#include "password_container.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>

#include <cstring>

#include <openssl/evp.h>

#include <zlib.h>

namespace seb::settingsinternal {
namespace {

QString sha256Hex(const QByteArray &data)
{
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QString hashPassword(const QString &password)
{
    return sha256Hex(password.toUtf8());
}

QByteArray inflateGzip(const QByteArray &data, QString *error)
{
    z_stream stream {};
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
    stream.avail_in = static_cast<uInt>(data.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        if (error) {
            *error = QStringLiteral("Failed to initialize gzip decompression.");
        }
        return {};
    }

    QByteArray output;
    char buffer[8192];
    int status = Z_OK;

    while (status == Z_OK) {
        stream.next_out = reinterpret_cast<Bytef *>(buffer);
        stream.avail_out = sizeof(buffer);
        status = inflate(&stream, Z_NO_FLUSH);

        if (status != Z_OK && status != Z_STREAM_END) {
            if (error) {
                *error = QStringLiteral("Failed to decompress gzip-compressed SEB data.");
            }
            inflateEnd(&stream);
            return {};
        }

        output.append(buffer, static_cast<int>(sizeof(buffer) - stream.avail_out));
    }

    inflateEnd(&stream);
    return output;
}

constexpr int kSebPrefixLength = 4;
constexpr int kSebCustomHeaderLength = 4;
constexpr int kSebMultipartLength = 8;
constexpr int kSebRncryptorHeaderLength = 2;
constexpr int kSebRncryptorSaltLength = 8;
constexpr int kSebRncryptorIvLength = 16;
constexpr int kSebRncryptorKeyLength = 32;
constexpr int kSebRncryptorIterations = 10000;
constexpr int kSebRncryptorVersion = 0x2;
constexpr int kSebRncryptorOptions = 0x1;

const QByteArray kPrefixPublicKey = "pkhs";
const QByteArray kPrefixPublicKeySymmetric = "phsk";
const QByteArray kPrefixPassword = "pswd";
const QByteArray kPrefixPasswordConfigureClient = "pwcc";
const QByteArray kPrefixPlain = "plnd";
const QByteArray kPrefixMultipart = "mphd";
const QByteArray kPrefixCustomHeader = "cmhd";

QByteArray readPrefix(const QByteArray &data)
{
    return data.left(kSebPrefixLength);
}

QByteArray deriveKey(const QString &password, const QByteArray &salt)
{
    const QByteArray passwordBytes = password.toUtf8();
    QByteArray key(kSebRncryptorKeyLength, Qt::Uninitialized);
    PKCS5_PBKDF2_HMAC_SHA1(
        passwordBytes.constData(),
        passwordBytes.size(),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        salt.size(),
        kSebRncryptorIterations,
        kSebRncryptorKeyLength,
        reinterpret_cast<unsigned char *>(key.data()));
    return key;
}

bool decryptPasswordBlock(const QByteArray &data, const QString &password, QByteArray *decrypted, QString *error)
{
    if (data.size() < kSebRncryptorHeaderLength + 2 * kSebRncryptorSaltLength + kSebRncryptorIvLength + 32) {
        if (error) {
            *error = QStringLiteral("The encrypted SEB payload is truncated.");
        }
        return false;
    }

    const unsigned char version = static_cast<unsigned char>(data.at(0));
    const unsigned char options = static_cast<unsigned char>(data.at(1));
    if (version != kSebRncryptorVersion || options != kSebRncryptorOptions) {
        if (error) {
            *error = QStringLiteral("Unsupported encrypted SEB format version.");
        }
        return false;
    }

    int offset = kSebRncryptorHeaderLength;
    const QByteArray encryptionSalt = data.mid(offset, kSebRncryptorSaltLength);
    offset += kSebRncryptorSaltLength;
    const QByteArray authenticationSalt = data.mid(offset, kSebRncryptorSaltLength);
    offset += kSebRncryptorSaltLength;
    const QByteArray iv = data.mid(offset, kSebRncryptorIvLength);
    offset += kSebRncryptorIvLength;

    const int hmacLength = 32;
    const QByteArray encryptedPayload = data.mid(offset, data.size() - offset - hmacLength);
    const QByteArray originalHmac = data.right(hmacLength);

    const QByteArray authenticationKey = deriveKey(password, authenticationSalt);
    const QByteArray encryptionKey = deriveKey(password, encryptionSalt);
    const QByteArray computedHmac = QMessageAuthenticationCode::hash(
        data.left(data.size() - hmacLength),
        authenticationKey,
        QCryptographicHash::Sha256);

    if (computedHmac != originalHmac) {
        if (error) {
            *error = QStringLiteral("Invalid password or corrupted encrypted SEB data.");
        }
        return false;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        if (error) {
            *error = QStringLiteral("Failed to decrypt the SEB payload.");
        }
        return false;
    }

    QByteArray clear(encryptedPayload.size() + kSebRncryptorIvLength, Qt::Uninitialized);
    int firstChunkLength = 0;
    int finalChunkLength = 0;
    const bool ok =
        EVP_DecryptInit_ex(
            ctx,
            EVP_aes_256_cbc(),
            nullptr,
            reinterpret_cast<const unsigned char *>(encryptionKey.constData()),
            reinterpret_cast<const unsigned char *>(iv.constData())) == 1 &&
        EVP_DecryptUpdate(
            ctx,
            reinterpret_cast<unsigned char *>(clear.data()),
            &firstChunkLength,
            reinterpret_cast<const unsigned char *>(encryptedPayload.constData()),
            encryptedPayload.size()) == 1 &&
        EVP_DecryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char *>(clear.data()) + firstChunkLength,
            &finalChunkLength) == 1;
    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        if (error) {
            *error = QStringLiteral("Failed to decrypt the SEB payload.");
        }
        return false;
    }

    clear.truncate(firstChunkLength + finalChunkLength);
    if (decrypted) {
        *decrypted = clear;
    }
    return true;
}

}  // namespace

bool looksLikeHtml(const QByteArray &raw)
{
    const QByteArray trimmed = raw.trimmed().left(256).toLower();
    return trimmed.startsWith("<!doctype html") || trimmed.startsWith("<html");
}

bool startsWithXml(const QByteArray &data)
{
    const QByteArray trimmed = data.trimmed();
    return trimmed.startsWith("<?xml") || trimmed.startsWith("<plist");
}

QByteArray unwrapSebContainer(
    QByteArray raw,
    QString *error,
    QStringList *warnings,
    const std::function<QString (bool)> &passwordProvider)
{
    while (raw.size() >= kSebPrefixLength) {
        const QByteArray prefix = readPrefix(raw);

        if (prefix == kPrefixMultipart) {
            if (raw.size() < kSebPrefixLength + kSebMultipartLength) {
                if (error) {
                    *error = QStringLiteral("Invalid multipart SEB header.");
                }
                return {};
            }

            qint64 firstPartLength = 0;
            memcpy(&firstPartLength, raw.constData() + kSebPrefixLength, sizeof(firstPartLength));
            raw = raw.mid(kSebPrefixLength + kSebMultipartLength, static_cast<int>(firstPartLength));
            if (warnings) {
                warnings->push_back(QStringLiteral("Ignored additional multipart SEB resources."));
            }
            continue;
        }

        if (prefix == kPrefixCustomHeader) {
            if (raw.size() < kSebPrefixLength + kSebCustomHeaderLength) {
                if (error) {
                    *error = QStringLiteral("Invalid custom-header SEB payload.");
                }
                return {};
            }

            qint32 headerLength = 0;
            memcpy(&headerLength, raw.constData() + kSebPrefixLength, sizeof(headerLength));
            raw = raw.mid(kSebPrefixLength + kSebCustomHeaderLength + headerLength);
            if (warnings) {
                warnings->push_back(QStringLiteral("Ignored unsupported custom SEB header data."));
            }
            continue;
        }

        if (prefix == kPrefixPlain) {
            return raw.mid(kSebPrefixLength);
        }

        if (prefix == kPrefixPassword || prefix == kPrefixPasswordConfigureClient) {
            const bool hashPasswordBeforeUse = prefix == kPrefixPasswordConfigureClient;
            const QByteArray encrypted = raw.mid(kSebPrefixLength);

            for (int attempt = 0; attempt < 5; ++attempt) {
                if (!passwordProvider) {
                    if (error) {
                        *error = QStringLiteral("This SEB file is encrypted and requires a password.");
                    }
                    return {};
                }

                const QString supplied = passwordProvider(hashPasswordBeforeUse);
                if (supplied.isNull() || supplied.isEmpty()) {
                    if (error) {
                        *error = QStringLiteral("Password entry was cancelled.");
                    }
                    return {};
                }

                const QString password = hashPasswordBeforeUse ? hashPassword(supplied).toUpper() : supplied;
                QByteArray decrypted;
                QString decryptError;
                if (decryptPasswordBlock(encrypted, password, &decrypted, &decryptError)) {
                    raw = decrypted;
                    if (raw.size() >= 2 &&
                        static_cast<unsigned char>(raw.at(0)) == 0x1f &&
                        static_cast<unsigned char>(raw.at(1)) == 0x8b) {
                        raw = inflateGzip(raw, &decryptError);
                        if (raw.isEmpty()) {
                            if (error) {
                                *error = decryptError;
                            }
                            return {};
                        }
                    }
                    break;
                }

                if (attempt == 4) {
                    if (error) {
                        *error = decryptError;
                    }
                    return {};
                }
            }

            continue;
        }

        if (prefix == kPrefixPublicKey || prefix == kPrefixPublicKeySymmetric) {
            if (error) {
                *error = QStringLiteral("Certificate-encrypted SEB files are not implemented yet on Linux.");
            }
            return {};
        }

        break;
    }

    return raw;
}

}  // namespace seb::settingsinternal
