#pragma once

#include "seb_settings.h"

#include <functional>
#include <memory>

#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

namespace seb::browser {
class RequestInterceptor;
}
namespace seb::applications {
class ApplicationManager;
class ExternalApplication;
}

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QTemporaryDir;
class QWidget;
QT_END_NAMESPACE

class BrowserWindow;

namespace seb::browser::contracts {
class IWebProfile;
class IEngineProvider;
}

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
    seb::browser::contracts::IWebProfile *profile() const;
    seb::browser::contracts::IEngineProvider *engineProvider() const;
    QUrl homeUrl() const;
    QUrl initialUrl() const;
    bool openSebResource(const QUrl &url, QWidget *parent) const;
    QList<BrowserWindow *> browserWindows() const;
    QList<seb::applications::ExternalApplication *> externalApplications() const;
    void registerBrowserWindow(BrowserWindow *window);
    void unregisterBrowserWindow(BrowserWindow *window);
    void notifyBrowserWindowStateChanged(BrowserWindow *window);

signals:
    void browserWindowsChanged();
    void externalApplicationsChanged();

public slots:
    void activateWindow(BrowserWindow *window);

private:
    void handleDownloadRequested(const QUrl &url, const QString &suggestedFilename, bool &accepted, QString &downloadDirectory);
    QString buildUserAgent() const;
    QString defaultDownloadDirectory() const;
    QString normalizeUrl(const QUrl &url) const;
    bool promptForPassword(QWidget *parent, const QString &title, const QString &message) const;

    seb::SebSettings settings_;
    QScopedPointer<seb::browser::RequestInterceptor> interceptor_;
    std::unique_ptr<seb::browser::contracts::IEngineProvider> engineProvider_;
    std::unique_ptr<seb::browser::contracts::IWebProfile> profile_;
    std::unique_ptr<QTemporaryDir> profileDirectory_;
    std::unique_ptr<QTemporaryDir> downloadDirectory_;
    std::unique_ptr<seb::applications::ApplicationManager> applicationManager_;
    ResourceOpener opener_;
    QList<BrowserWindow *> browserWindows_;
};
