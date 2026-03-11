#pragma once

#include "seb_settings.h"

#include <functional>
#include <memory>

#include <QObject>
#include <QScopedPointer>

namespace seb::browser {
class RequestInterceptor;
}

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QTemporaryDir;
class QUrl;
class QWidget;
class QWebEngineDownloadRequest;
class QWebEngineProfile;
QT_END_NAMESPACE

class BrowserWindow;

class SebSession : public QObject
{
    Q_OBJECT

public:
    using ResourceOpener = std::function<bool(const QString &, QWidget *)>;

    explicit SebSession(const seb::SebSettings &settings, ResourceOpener opener, QObject *parent = nullptr);
    ~SebSession() override;

    BrowserWindow *createWindow(const QUrl &url, bool isMainWindow);
    bool applyProxyAuthentication(const QString &proxyHost, QAuthenticator *authenticator) const;
    bool confirmReload(QWidget *parent) const;
    bool confirmQuitUrl(QWidget *parent) const;
    bool isPopupAllowed(const QUrl &opener, const QUrl &target, bool *openInSameWindow) const;
    bool isQuitUrl(const QUrl &url) const;
    bool promptForHomeNavigation(QWidget *parent) const;
    bool requestApplicationQuit(QWidget *parent, const QString &reason) const;
    const seb::SebSettings &settings() const;
    QWebEngineProfile *profile() const;
    QUrl homeUrl() const;
    QUrl initialUrl() const;
    bool openSebResource(const QUrl &url, QWidget *parent) const;

private:
    void handleDownloadRequested(QWebEngineDownloadRequest *download);
    QString buildUserAgent() const;
    QString defaultDownloadDirectory() const;
    QString normalizeUrl(const QUrl &url) const;
    bool promptForPassword(QWidget *parent, const QString &title, const QString &message) const;

    seb::SebSettings settings_;
    QScopedPointer<QWebEngineProfile> profile_;
    QScopedPointer<seb::browser::RequestInterceptor> interceptor_;
    std::unique_ptr<QTemporaryDir> profileDirectory_;
    std::unique_ptr<QTemporaryDir> downloadDirectory_;
    ResourceOpener opener_;
};
