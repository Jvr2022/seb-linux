#pragma once

#include "seb_settings.h"

#include <memory>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QCloseEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QLineEdit;
class QToolBar;
class QWidget;
class QUrl;
QT_END_NAMESPACE

class SebSession;
class SebTaskbar;

class BrowserWindow;
namespace seb::browser::engine {
class BrowserView;
}

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

private:
    void applyWindowFlags();
    void applyWindowGeometry();
    void configureToolbar();
    void configureShortcuts();
    void findInPage();
    void navigateHome();
    void openDevTools();
    void handleNewWindowRequest(const QUrl &targetUrl, const QUrl &openerUrl);
    void reloadPage();
    void updateAddressBar(const QUrl &url);
    void notifyTaskbarStateChanged();

    SebSession &session_;
    seb::WindowSettings windowSettings_;
    std::unique_ptr<seb::browser::engine::BrowserView> view_;
    QWidget *contentContainer_ = nullptr;
    QToolBar *toolbar_ = nullptr;
    QLineEdit *addressBar_ = nullptr;
    SebTaskbar *taskbar_ = nullptr;
    bool isMainWindow_ = false;
};
