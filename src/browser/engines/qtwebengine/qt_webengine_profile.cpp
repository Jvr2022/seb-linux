#include "../../webengine_compat.h"
#include "qt_webengine_profile.h"
#include "browser/webengine_environment.h"


#if SEB_HAS_QTWEBENGINE
#include <QWebEngineCookieStore>
#include <QWebEngineDownloadRequest>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
#include <QDir>
#include <utility>

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
    explicit NativeRequestInterceptor(std::shared_ptr<contracts::IRequestInterceptor> abstractInterceptor, QObject *parent = nullptr)
        : QWebEngineUrlRequestInterceptor(parent)
        , abstractInterceptor_(std::move(abstractInterceptor))
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
    std::shared_ptr<contracts::IRequestInterceptor> abstractInterceptor_;
};

}  // namespace

QtWebEngineProfile::QtWebEngineProfile(
    const QString &storageName,
    QObject *parent)
    : IWebProfile(parent)
{
    freezeWebEngineEnvironment();
    profile_ = storageName.isEmpty()
        ? new QWebEngineProfile(this)
        : new QWebEngineProfile(storageName, this);
    defaultUserAgent_ = profile_->httpUserAgent();
    cacheTimeout_.setSingleShot(true);
    cacheTimeout_.setInterval(30000);
    connect(&cacheTimeout_, &QTimer::timeout, this, [this] {
        const auto callbacks = std::exchange(cacheWaiters_, {});
        for (const auto &callback : callbacks) if (callback) callback(false);
    });
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(profile_, &QWebEngineProfile::clearHttpCacheCompleted, this, [this] {
        cacheTimeout_.stop();
        clearingCache_ = false;
        const auto callbacks = std::exchange(cacheWaiters_, {});
        for (const auto &callback : callbacks) if (callback) callback(true);
    });
#endif

    if (profile_->cookieStore()) {
        connect(profile_->cookieStore(), &QWebEngineCookieStore::cookieAdded,
                this, &contracts::IWebProfile::cookieAdded);
    }

    // Forward native download requests to the abstraction
    connect(profile_, &QWebEngineProfile::downloadRequested, this, [this](auto *download) {
        downloads_.insert(download);
        connect(download, &QObject::destroyed, this, [this, download] { downloads_.remove(download); });
        contracts::DownloadRequest request;
        request.id = QUuid::createUuid();
        request.url = download->url();
        request.suggestedFileName = download->downloadFileName();
        request.mimeType = download->mimeType();
        emit this->downloadRequested(request);
        
        if (!request.accepted) {
            download->cancel();
        } else {
            if (!request.targetDirectory.isEmpty()) {
                download->setDownloadDirectory(request.targetDirectory);
            }
            if (!request.targetFileName.isEmpty()) {
                download->setDownloadFileName(request.targetFileName);
            }
            const QUuid requestId = request.id;
            const QUrl requestUrl = request.url;
            auto emitted = std::make_shared<bool>(false);
            connect(download, &QWebEngineDownloadRequest::isFinishedChanged, this, [this, download, requestId, requestUrl, emitted]() {
                if (*emitted) {
                    return;
                }
                if (download->state() == QWebEngineDownloadRequest::DownloadInProgress) {
                    return;
                }
                *emitted = true;
                contracts::DownloadResult result;
                result.id = requestId;
                result.url = requestUrl;
                if (download->state() == QWebEngineDownloadRequest::DownloadCompleted) {
                    result.localPath = QDir(download->downloadDirectory()).filePath(download->downloadFileName());
                    result.success = true;
                } else {
                    result.success = false;
                }
                emit this->downloadFinished(result);
            });
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

QString QtWebEngineProfile::httpUserAgent() const
{
    return profile_->httpUserAgent();
}

QString QtWebEngineProfile::defaultHttpUserAgent() const
{
    return defaultUserAgent_;
}

void QtWebEngineProfile::setHttpUserAgent(const QString &userAgent)
{
    profile_->setHttpUserAgent(userAgent);
}

void QtWebEngineProfile::setHttpAcceptLanguage(const QString &language)
{
    profile_->setHttpAcceptLanguage(language);
}

void QtWebEngineProfile::setPersistSessionCookies(bool persist)
{
    profile_->setPersistentCookiesPolicy(
        persist
            ? QWebEngineProfile::ForcePersistentCookies
            : QWebEngineProfile::AllowPersistentCookies);
}

void QtWebEngineProfile::setUrlRequestInterceptor(std::shared_ptr<contracts::IRequestInterceptor> interceptor)
{
    auto *previous = nativeInterceptor_;
    nativeInterceptor_ = interceptor ? new NativeRequestInterceptor(std::move(interceptor), this) : nullptr;
    profile_->setUrlRequestInterceptor(nativeInterceptor_);
    if (previous) previous->deleteLater();
}

void QtWebEngineProfile::deleteAllCookies()
{
    if (profile_->cookieStore()) {
        profile_->cookieStore()->deleteAllCookies();
    }
}

void QtWebEngineProfile::clearHttpCache(std::function<void(bool)> done)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    if (done) cacheWaiters_.append(std::move(done));
    if (clearingCache_) return;
    clearingCache_ = true;
    cacheTimeout_.start();
    profile_->clearHttpCache();
#else
    profile_->clearHttpCache();
    if (done) QTimer::singleShot(0, this, [done] { done(false); });
#endif
}

void QtWebEngineProfile::cancelDownloads()
{
    const auto downloads = downloads_;
    for (auto *download : downloads) if (!download->isFinished()) download->cancel();
}

}  // namespace seb::browser
#endif
