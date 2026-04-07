#pragma once

#if SEB_HAS_QTWEBENGINE

#include "browser/contracts/i_request_interceptor.h"
#include "browser/contracts/i_webprofile.h"
#include <QWebEngineProfile>

namespace seb::browser {

class QtWebEngineProfile : public contracts::IWebProfile
{
    Q_OBJECT

public:
    explicit QtWebEngineProfile(QObject *parent = nullptr);
    ~QtWebEngineProfile() override = default;

    QWebEngineProfile *nativeProfile() const { return profile_; }

    void setPersistentStoragePath(const QString &path) override;
    void setCachePath(const QString &path) override;
    void setDownloadPath(const QString &path) override;
    
    void setPdfViewerEnabled(bool enabled) override;
    void setSpellCheckEnabled(bool enabled) override;
    void setSpellCheckLanguages(const QStringList &languages) override;
    void setHttpUserAgent(const QString &userAgent) override;
    void setDevBypass(bool enabled) override { devBypass_ = enabled; }
    bool devBypass() const { return devBypass_; }

    void setUrlRequestInterceptor(contracts::IRequestInterceptor *interceptor) override;
    void deleteAllCookies() override;

private:
    QWebEngineProfile *profile_ = nullptr;
    bool devBypass_ = false;
};

}  // namespace seb::browser

#endif
