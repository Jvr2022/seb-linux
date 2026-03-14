#pragma once

#include <QString>
#include <QStringList>

namespace seb::settings::applications {

struct WhitelistApplication
{
    bool AllowCustomPath = false;
    bool AllowRunning = false;
    QStringList Arguments;
    bool AutoStart = false;
    bool AutoTerminate = false;
    QString Description;
    QString DisplayName;
    QString ExecutableName;
    QString ExecutablePath;
    QString Identifier;
    QString OriginalName;
    bool ShowInShell = true;
    QString Signature;
};

}  // namespace seb::settings::applications
