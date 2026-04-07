#include "password_encryption.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

namespace seb::configuration::cryptography {

namespace {
constexpr int kIterations = 10000;
constexpr int kKeySize = 32;
constexpr int kSaltSize = 8;
constexpr int kIvSize = 16;
constexpr int kVersion = 2;
constexpr int kOptions = 1;
constexpr int kHmacSize = 32;
} // namespace

contracts::LoadStatus PasswordEncryption::decrypt(const QByteArray &data,
                                                  const QString &password,
                                                  QByteArray &decrypted) const {
  if (password.isNull())
    return contracts::LoadStatus::PasswordNeeded;
  if (data.size() < (2 + 2 * kSaltSize + kIvSize + kHmacSize))
    return contracts::LoadStatus::InvalidData;

  const char *ptr = data.constData();
  if (static_cast<unsigned char>(ptr[0]) != kVersion ||
      static_cast<unsigned char>(ptr[1]) != kOptions)
    return contracts::LoadStatus::InvalidData;

  QByteArray encSalt = data.mid(2, kSaltSize);
  QByteArray authSalt = data.mid(2 + kSaltSize, kSaltSize);
  QByteArray iv = data.mid(2 + 2 * kSaltSize, kIvSize);
  QByteArray payload =
      data.mid(2 + 2 * kSaltSize + kIvSize,
               data.size() - (2 + 2 * kSaltSize + kIvSize + kHmacSize));
  QByteArray originalHmac = data.right(kHmacSize);

  QByteArray encryptionKey(kKeySize, 0);
  QByteArray authenticationKey(kKeySize, 0);

  PKCS5_PBKDF2_HMAC_SHA1(
      password.toUtf8().constData(), -1,
      reinterpret_cast<const unsigned char *>(encSalt.constData()), kSaltSize,
      kIterations, kKeySize,
      reinterpret_cast<unsigned char *>(encryptionKey.data()));
  PKCS5_PBKDF2_HMAC_SHA1(
      password.toUtf8().constData(), -1,
      reinterpret_cast<const unsigned char *>(authSalt.constData()), kSaltSize,
      kIterations, kKeySize,
      reinterpret_cast<unsigned char *>(authenticationKey.data()));

  // Verify HMAC
  unsigned char computedHmac[kHmacSize];
  if (!HMAC(EVP_sha256(), authenticationKey.constData(), kKeySize, 
            reinterpret_cast<const unsigned char*>(data.constData()), 
            data.size() - kHmacSize, computedHmac, nullptr)) {
      return contracts::LoadStatus::UnexpectedError;
  }

  if (CRYPTO_memcmp(computedHmac, originalHmac.constData(), kHmacSize) != 0) {
      return contracts::LoadStatus::InvalidData;
  }

  // Decrypt
  decrypted.resize(payload.size());
  int outLen = 0;
  int finalLen = 0;
  
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return contracts::LoadStatus::UnexpectedError;

  auto cleanup = [ctx, &decrypted]() {
      EVP_CIPHER_CTX_free(ctx);
      decrypted.clear();
  };

  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                         reinterpret_cast<const unsigned char*>(encryptionKey.constData()), 
                         reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
      cleanup();
      return contracts::LoadStatus::UnexpectedError;
  }

  if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted.data()), &outLen, 
                        reinterpret_cast<const unsigned char*>(payload.constData()), 
                        payload.size()) != 1) {
      cleanup();
      return contracts::LoadStatus::InvalidData;
  }

  if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted.data()) + outLen, &finalLen) != 1) {
      cleanup();
      return contracts::LoadStatus::InvalidData;
  }

  EVP_CIPHER_CTX_free(ctx);
  decrypted.resize(outLen + finalLen);
  return contracts::LoadStatus::Success;
}

contracts::SaveStatus PasswordEncryption::encrypt(const QByteArray &data,
                                                  const QString &password,
                                                  QByteArray &encrypted) const {
  if (password.isNull())
    return contracts::SaveStatus::UnexpectedError;

  QByteArray encSalt(kSaltSize, 0);
  QByteArray authSalt(kSaltSize, 0);
  QByteArray iv(kIvSize, 0);
  RAND_bytes(reinterpret_cast<unsigned char *>(encSalt.data()), kSaltSize);
  RAND_bytes(reinterpret_cast<unsigned char *>(authSalt.data()), kSaltSize);
  RAND_bytes(reinterpret_cast<unsigned char *>(iv.data()), kIvSize);

  QByteArray encryptionKey(kKeySize, 0);
  QByteArray authenticationKey(kKeySize, 0);
  PKCS5_PBKDF2_HMAC_SHA1(
      password.toUtf8().constData(), -1,
      reinterpret_cast<const unsigned char *>(encSalt.constData()), kSaltSize,
      kIterations, kKeySize,
      reinterpret_cast<unsigned char *>(encryptionKey.data()));
  PKCS5_PBKDF2_HMAC_SHA1(
      password.toUtf8().constData(), -1,
      reinterpret_cast<const unsigned char *>(authSalt.constData()), kSaltSize,
      kIterations, kKeySize,
      reinterpret_cast<unsigned char *>(authenticationKey.data()));

  encrypted.append(static_cast<char>(kVersion));
  encrypted.append(static_cast<char>(kOptions));
  encrypted.append(encSalt);
  encrypted.append(authSalt);
  encrypted.append(iv);

  QByteArray cipherData(data.size() + kIvSize, 0);
  int outLen = 0;
  int finalLen = 0;
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_EncryptInit_ex(
      ctx, EVP_aes_256_cbc(), nullptr,
      reinterpret_cast<const unsigned char *>(encryptionKey.constData()),
      reinterpret_cast<const unsigned char *>(iv.constData()));
  EVP_EncryptUpdate(
      ctx, reinterpret_cast<unsigned char *>(cipherData.data()), &outLen,
      reinterpret_cast<const unsigned char *>(data.constData()), data.size());
  EVP_EncryptFinal_ex(
      ctx, reinterpret_cast<unsigned char *>(cipherData.data()) + outLen,
      &finalLen);
  EVP_CIPHER_CTX_free(ctx);
  cipherData.resize(outLen + finalLen);
  encrypted.append(cipherData);

  unsigned int hmacLen = 0;
  unsigned char hmac[kHmacSize];
  HMAC(EVP_sha256(), authenticationKey.constData(), kKeySize,
       reinterpret_cast<const unsigned char *>(encrypted.constData()),
       encrypted.size(), hmac, &hmacLen);
  encrypted.append(reinterpret_cast<const char *>(hmac), kHmacSize);

  return contracts::SaveStatus::Success;
}

} // namespace seb::configuration::cryptography
