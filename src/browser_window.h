#pragma once

#include "seb_settings.h"

#include <QMainWindow>


#include <memory>

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

namespace seb::browser::contracts {
class IWebView;
}

class SebSession;
class SebTaskbar;

class BrowserWindow;


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
    void focusOutEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applyWindowFlags();
    void applyWindowGeometry();
    void configureToolbar();
    void configureShortcuts();
    void findInPage();
    void navigateHome();
    void handleNewWindowRequest(const QUrl &url, bool &handled);
    void reloadPage();
    void updateAddressBar(const QUrl &url);
    void notifyTaskbarStateChanged();

    SebSession &session_;
    seb::WindowSettings windowSettings_;
    std::unique_ptr<seb::browser::contracts::IWebView> view_;
    QWidget *contentContainer_ = nullptr;
    QToolBar *toolbar_ = nullptr;
    QLineEdit *addressBar_ = nullptr;
    SebTaskbar *taskbar_ = nullptr;
    bool isMainWindow_ = false;
};
