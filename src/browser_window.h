#pragma once

#include "seb_settings.h"

#include <QMainWindow>
#include <QWebEnginePage>

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
class QWebEngineNewWindowRequest;
class QWebEngineView;
QT_END_NAMESPACE

class SebSession;
class SebTaskbar;

class BrowserWindow;

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

    QWebEnginePage *page() const;
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
    void handleNewWindowRequest(QWebEngineNewWindowRequest &request);
    void reloadPage();
    void updateAddressBar(const QUrl &url);
    void notifyTaskbarStateChanged();

    SebSession &session_;
    seb::WindowSettings windowSettings_;
    QWebEngineView *view_ = nullptr;
    BrowserPage *page_ = nullptr;
    QWidget *contentContainer_ = nullptr;
    QToolBar *toolbar_ = nullptr;
    QLineEdit *addressBar_ = nullptr;
    SebTaskbar *taskbar_ = nullptr;
    bool isMainWindow_ = false;
};
