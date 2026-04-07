#include "../../webengine_compat.h"
#include "qt_webengine_profile.h"


#if SEB_HAS_QTWEBENGINE
#include <QWebEngineCookieStore>
#include <QWebEngineDownloadRequest>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>

namespace seb::browser {

namespace {

contracts::ResourceType mapResourceType(QWebEngineUrlRequestInfo::ResourceType type)
{
    switch (type) {
    case QWebEngineUrlRequestInfo::ResourceTypeMainFrame:
        return contracts::ResourceType::MainFrame;
    case QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadMainFrame:
        return contracts::ResourceType::NavigationPreloadMainFrame;
    case QWebEngineUrlRequestInfo::ResourceTypeSubFrame:
        return contracts::ResourceType::SubFrame;
    default:
        return contracts::ResourceType::Other;
    }
}

class NativeRequestWrapper : public contracts::IRequest
{
public:
    explicit NativeRequestWrapper(QWebEngineUrlRequestInfo &info) : info_(info) {}

    QUrl requestUrl() const override { return info_.requestUrl(); }
    QUrl firstPartyUrl() const override { return info_.firstPartyUrl(); }
    contracts::ResourceType resourceType() const override { return mapResourceType(info_.resourceType()); }

    void block(bool shouldBlock) override { info_.block(shouldBlock); }
    void redirect(const QUrl &url) override { info_.redirect(url); }
    void setHttpHeader(const QByteArray &name, const QByteArray &value) override { info_.setHttpHeader(name, value); }

private:
    QWebEngineUrlRequestInfo &info_;
};

class NativeRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    explicit NativeRequestInterceptor(contracts::IRequestInterceptor *abstractInterceptor, QObject *parent = nullptr)
        : QWebEngineUrlRequestInterceptor(parent)
        , abstractInterceptor_(abstractInterceptor)
    {
    }

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        if (abstractInterceptor_) {
            NativeRequestWrapper request(info);
            abstractInterceptor_->interceptRequest(request);
        }
    }

private:
    contracts::IRequestInterceptor *abstractInterceptor_;
};

}  // namespace

QtWebEngineProfile::QtWebEngineProfile(QObject *parent)
    : IWebProfile(parent)
{
    profile_ = new QWebEngineProfile(this);

    // Forward native download requests to the abstraction
    connect(profile_, &QWebEngineProfile::downloadRequested, this, [this](auto *download) {
        bool accepted = false;
        QString downloadDir;
        emit this->downloadRequested(download->url(), download->downloadFileName(), accepted, downloadDir);
        
        if (!accepted) {
            download->cancel();
        } else {
            if (!downloadDir.isEmpty()) {
                download->setDownloadDirectory(downloadDir);
            }
            download->accept();
        }
    });
}

void QtWebEngineProfile::setPersistentStoragePath(const QString &path)
{
    profile_->setPersistentStoragePath(path);
}

void QtWebEngineProfile::setCachePath(const QString &path)
{
    profile_->setCachePath(path);
}

void QtWebEngineProfile::setDownloadPath(const QString &path)
{
    profile_->setDownloadPath(path);
}

void QtWebEngineProfile::setPdfViewerEnabled(bool enabled)
{
    profile_->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, enabled);
}

void QtWebEngineProfile::setSpellCheckEnabled(bool enabled)
{
    profile_->setSpellCheckEnabled(enabled);
}

void QtWebEngineProfile::setSpellCheckLanguages(const QStringList &languages)
{
    profile_->setSpellCheckLanguages(languages);
}

void QtWebEngineProfile::setHttpUserAgent(const QString &userAgent)
{
    profile_->setHttpUserAgent(userAgent);
}

void QtWebEngineProfile::setUrlRequestInterceptor(contracts::IRequestInterceptor *interceptor)
{
    if (!interceptor) {
        profile_->setUrlRequestInterceptor(nullptr);
        return;
    }

    auto *nativeInterceptor = new NativeRequestInterceptor(interceptor, this);
    profile_->setUrlRequestInterceptor(nativeInterceptor);
}

void QtWebEngineProfile::deleteAllCookies()
{
    if (profile_->cookieStore()) {
        profile_->cookieStore()->deleteAllCookies();
    }
}

}  // namespace seb::browser
#endif
