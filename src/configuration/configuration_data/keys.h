#pragma once

#include <QString>

namespace seb::configuration::configuration_data::keys {

namespace ConfigurationFile {
inline const QString KeepClientConfigEncryption = QStringLiteral("sebConfigPurpose");
}

namespace Meta {
inline const QString RawSebData = QStringLiteral("__rawSebData");
inline const QString SourceText = QStringLiteral("__sourceText");
}

}  // namespace seb::configuration::configuration_data::keys
