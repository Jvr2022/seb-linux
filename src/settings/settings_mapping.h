#pragma once

#include "../seb_settings.h"
#include <QVariant>

namespace seb::settingsinternal {

bool hasStructuredSections(const QVariantMap &data);
QByteArray readBytes(const QVariant &value);
void applyStructuredSettings(const QVariantMap &data, SebSettings &settings);
void applyRawSettings(const QVariantMap &data, SebSettings &settings);

}  // namespace seb::settingsinternal
