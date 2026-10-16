#pragma once

#include <QUrl>

namespace seb::server {

class Sanitizer
{
public:
    static QUrl baseAddress(const QString &serverUrl);
    static QString apiEndpoint(const QString &endpoint);
    static QUrl resolve(const QUrl &base, const QString &endpoint);
};

}  // namespace seb::server
