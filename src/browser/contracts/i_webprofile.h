#pragma once

#include <QObject>
#include <QtGlobal>
#include <QString>
#include <QStringList>

#include "download_request.h"

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

namespace seb::browser::contracts {
class IRequestInterceptor;
}

namespace seb::browser::contracts {

class IWebProfile : public QObject
{
    Q_OBJECT

public:
    explicit IWebProfile(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IWebProfile() = default;

    virtual void setPersistentStoragePath(const QString &path) = 0;
    virtual void setCachePath(const QString &path) = 0;
    virtual void setDownloadPath(const QString &path) = 0;
    
    // Settings equivalent to QWebEngineSettings
    virtual void setPdfViewerEnabled(bool enabled) = 0;
    virtual void setSpellCheckEnabled(bool enabled) = 0;
    virtual void setSpellCheckLanguages(const QStringList &languages) = 0;
    virtual QString defaultHttpUserAgent() const = 0;
    virtual QString httpUserAgent() const = 0;
    virtual void setHttpUserAgent(const QString &userAgent) = 0;
    virtual void setHttpAcceptLanguage(const QString &language) = 0;
    virtual void setPersistSessionCookies(bool persist) = 0;
    virtual void setDevBypass(bool enabled) = 0;
    virtual void setUrlRequestInterceptor(IRequestInterceptor *interceptor) = 0;
    virtual void deleteAllCookies() = 0;
    virtual void clearHttpCache() = 0;

signals:
    // Simple download abstraction. A full download interceptor would have its own class.
    void downloadRequested(DownloadRequest &request);
    void downloadFinished(const QString &localPath);
};

}  // namespace seb::browser::contracts
