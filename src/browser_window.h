#pragma once

#include "browser/webengine_compat.h"
#include "seb_settings.h"

#include <QMainWindow>
#if SEB_HAS_QTWEBENGINE
#include <QWebEnginePage>
#endif

QT_BEGIN_NAMESPACE
#if SEB_HAS_QTWEBENGINE
class QAuthenticator;
#endif
class QCloseEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QLineEdit;
class QTextBrowser;
class QToolBar;
class QWidget;
class QUrl;
#if SEB_HAS_QTWEBENGINE
class QWebEngineNewWindowRequest;
class QWebEngineView;
#endif
QT_END_NAMESPACE

class SebSession;
class SebTaskbar;

class BrowserWindow;

#if SEB_HAS_QTWEBENGINE
class BrowserPage : public QWebEnginePage
{
public:
    BrowserPage(SebSession &session, BrowserWindow *window);

protected:
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
    QStringList chooseFiles(
        QWebEnginePage::FileSelectionMode mode,
        const QStringList &oldFiles,
        const QStringList &acceptedMimeTypes) override;

private:
    SebSession &session_;
    BrowserWindow *window_;
};
#endif

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    BrowserWindow(
        SebSession &session,
        const seb::WindowSettings &windowSettings,
        const QUrl &initialUrl,
        bool isMainWindow);
    ~BrowserWindow() override;

#if SEB_HAS_QTWEBENGINE
    QWebEnginePage *page() const;
#endif
    QUrl currentUrl() const;
    bool isMainWindow() const;
    bool shouldAllowNavigation(const QUrl &url);
    QString taskbarIconPath() const;
    QString taskbarTitle() const;

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    void applyWindowFlags();
    void applyWindowGeometry();
    void configureToolbar();
    void configureFallbackView(const QUrl &initialUrl);
    void configureShortcuts();
    void findInPage();
    void navigateHome();
    void openDevTools();
#if SEB_HAS_QTWEBENGINE
    void handleNewWindowRequest(QWebEngineNewWindowRequest &request);
#endif
    void reloadPage();
    void updateAddressBar(const QUrl &url);
    void notifyTaskbarStateChanged();

    SebSession &session_;
    seb::WindowSettings windowSettings_;
#if SEB_HAS_QTWEBENGINE
    QWebEngineView *view_ = nullptr;
    BrowserPage *page_ = nullptr;
#else
    QTextBrowser *fallbackView_ = nullptr;
    QUrl fallbackUrl_;
#endif
    QWidget *contentContainer_ = nullptr;
    QToolBar *toolbar_ = nullptr;
    QLineEdit *addressBar_ = nullptr;
    SebTaskbar *taskbar_ = nullptr;
    bool isMainWindow_ = false;
};
