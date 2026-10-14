#pragma once

#include <QByteArray>

namespace seb::configuration::data_formats::BinaryBlock {

inline const QByteArray Password = QByteArrayLiteral("pswd");
inline const QByteArray PasswordConfigureClient = QByteArrayLiteral("pwcc");
inline const QByteArray PlainData = QByteArrayLiteral("plnd");
inline const QByteArray PublicKey = QByteArrayLiteral("pkhs");
inline const QByteArray PublicKeySymmetric = QByteArrayLiteral("phsk");

}  // namespace seb::configuration::data_formats::BinaryBlock
