#pragma once

#include "../seb_settings.h"
#include <QVariant>

namespace seb::settingsinternal {

QString calculateConfigurationKey(const QVariantMap &data);
QByteArray inflateGzip(const QByteArray &data, QString *error);
QVariantMap parseJson(const QByteArray &raw, QString *error);
QVariantMap parsePlist(const QByteArray &raw, QString *error);

}  // namespace seb::settingsinternal
