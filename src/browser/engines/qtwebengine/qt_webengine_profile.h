#pragma once

#if SEB_HAS_QTWEBENGINE

#include "browser/contracts/i_request_interceptor.h"
#include "browser/contracts/i_webprofile.h"
#include <QWebEngineProfile>
#include <QSet>
#include <QTimer>
class QWebEngineDownloadRequest;
class QWebEngineUrlRequestInterceptor;

namespace seb::browser {

class QtWebEngineProfile : public contracts::IWebProfile
{
    Q_OBJECT

public:
    explicit QtWebEngineProfile(
        const QString &storageName = QString(),
        QObject *parent = nullptr);
    ~QtWebEngineProfile() override = default;

    QWebEngineProfile *nativeProfile() const { return profile_; }

    void setPersistentStoragePath(const QString &path) override;
    void setCachePath(const QString &path) override;
    void setDownloadPath(const QString &path) override;
    
    void setPdfViewerEnabled(bool enabled) override;
    void setSpellCheckEnabled(bool enabled) override;
    void setSpellCheckLanguages(const QStringList &languages) override;
    QString defaultHttpUserAgent() const override;
    QString httpUserAgent() const override;
    void setHttpUserAgent(const QString &userAgent) override;
    void setHttpAcceptLanguage(const QString &language) override;
    void setPersistSessionCookies(bool persist) override;

    void setUrlRequestInterceptor(std::shared_ptr<contracts::IRequestInterceptor> interceptor) override;
    void deleteAllCookies() override;
    void clearHttpCache(std::function<void(bool)> done = {}) override;
    void cancelDownloads() override;

private:
    QWebEngineProfile *profile_ = nullptr;
    QString defaultUserAgent_;
    QSet<QWebEngineDownloadRequest *> downloads_;
    QTimer cacheTimeout_;
    QList<std::function<void(bool)>> cacheWaiters_;
    bool clearingCache_ = false;
    QWebEngineUrlRequestInterceptor *nativeInterceptor_ = nullptr;
};

}  // namespace seb::browser

#endif
