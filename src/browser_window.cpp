#include "browser_window.h"

#include "seb_session.h"
#include "ui/seb_taskbar.h"

#include <QAction>
#include <QApplication>
#include <QAuthenticator>
#include <QCloseEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QScreen>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEngineNewWindowRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

BrowserPage::BrowserPage(SebSession &session, BrowserWindow *window)
    : QWebEnginePage(session.profile(), window)
    , session_(session)
    , window_(window)
{
}

bool BrowserPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (isMainFrame && !window_->shouldAllowNavigation(url)) {
        return false;
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QStringList BrowserPage::chooseFiles(
    QWebEnginePage::FileSelectionMode mode,
    const QStringList &oldFiles,
    const QStringList &acceptedMimeTypes)
{
    if (!session_.settings().browser.allowUploads) {
        return {};
    }
    return QWebEnginePage::chooseFiles(mode, oldFiles, acceptedMimeTypes);
}

BrowserWindow::BrowserWindow(
    SebSession &session,
    const seb::WindowSettings &windowSettings,
    const QUrl &initialUrl,
    bool isMainWindow)
    : session_(session)
    , windowSettings_(windowSettings)
    , isMainWindow_(isMainWindow)
{
    applyWindowFlags();

    contentContainer_ = new QWidget(this);
    auto *contentLayout = new QVBoxLayout(contentContainer_);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    view_ = new QWebEngineView(contentContainer_);
    page_ = new BrowserPage(session_, this);
    view_->setPage(page_);
    contentLayout->addWidget(view_, 1);

    if (isMainWindow_ && session_.settings().taskbar.enableTaskbar) {
        taskbar_ = new SebTaskbar(session_, session_.settings(), contentContainer_);
        contentLayout->addWidget(taskbar_);
    }
    setCentralWidget(contentContainer_);

    view_->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
    view_->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);

    if (!isMainWindow_) {
        configureToolbar();
    }
    configureShortcuts();
    applyWindowGeometry();

    connect(view_, &QWebEngineView::urlChanged, this, &BrowserWindow::updateAddressBar);
    connect(view_, &QWebEngineView::titleChanged, this, [this](const QString &title) {
        const QString resolvedTitle = title.trimmed().isEmpty() ? QStringLiteral("SEB Linux") : title;
        setWindowTitle(resolvedTitle);
        if (taskbar_) {
            taskbar_->setWindowTitleText(resolvedTitle);
        }
        notifyTaskbarStateChanged();
    });
    connect(page_, &QWebEnginePage::newWindowRequested, this, &BrowserWindow::handleNewWindowRequest);
    connect(
        page_,
        &QWebEnginePage::proxyAuthenticationRequired,
        this,
        [this](const QUrl &, QAuthenticator *authenticator, const QString &proxyHost) {
            session_.applyProxyAuthentication(proxyHost, authenticator);
        });
    if (taskbar_) {
        connect(taskbar_, &SebTaskbar::quitRequested, this, [this] {
            if (isMainWindow_ &&
                session_.requestApplicationQuit(
                    this,
                    QStringLiteral("Enter the quit password to close this exam session."))) {
                QApplication::quit();
            }
        });
    }

    if (initialUrl.isValid()) {
        view_->setUrl(initialUrl);
    }

    session_.registerBrowserWindow(this);
    notifyTaskbarStateChanged();
}

BrowserWindow::~BrowserWindow()
{
    session_.unregisterBrowserWindow(this);
}

QWebEnginePage *BrowserWindow::page() const
{
    return page_;
}

bool BrowserWindow::isMainWindow() const
{
    return isMainWindow_;
}

bool BrowserWindow::shouldAllowNavigation(const QUrl &url)
{
    if (!url.isValid()) {
        return true;
    }

    const QString scheme = url.scheme().toLower();
    if (scheme == QStringLiteral("seb") || scheme == QStringLiteral("sebs") ||
        url.path().endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive)) {
        session_.openSebResource(url, this);
        return false;
    }

    if (session_.isQuitUrl(url)) {
        if (session_.settings().browser.resetOnQuitUrl) {
            const QUrl home = session_.homeUrl();
            if (home.isValid()) {
                view_->setUrl(home);
            }
            return false;
        }

        if (session_.settings().browser.confirmQuitUrl && !session_.confirmQuitUrl(this)) {
            return false;
        }

        if (session_.requestApplicationQuit(
                this,
                QStringLiteral("Enter the quit password to close this exam session."))) {
            QApplication::quit();
        }
        return false;
    }

    return true;
}

QString BrowserWindow::taskbarIconPath() const
{
    return QStringLiteral(":/assets/icons/safe-exam-browser.png");
}

QString BrowserWindow::taskbarTitle() const
{
    const QString title = windowTitle().trimmed();
    return title.isEmpty() ? QStringLiteral("Safe Exam Browser") : title;
}

void BrowserWindow::changeEvent(QEvent *event)
{
    if (!windowSettings_.allowMinimize &&
        event->type() == QEvent::WindowStateChange &&
        isMinimized()) {
        QTimer::singleShot(0, this, [this] {
            showNormal();
            raise();
            activateWindow();
        });
    }

    QMainWindow::changeEvent(event);
    notifyTaskbarStateChanged();
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if (!isMainWindow_) {
        event->accept();
        return;
    }

    if (session_.requestApplicationQuit(
            this,
            QStringLiteral("Enter the quit password to close this exam session."))) {
        event->accept();
        return;
    }

    event->ignore();
}

void BrowserWindow::keyPressEvent(QKeyEvent *event)
{
    const bool ctrl = event->modifiers().testFlag(Qt::ControlModifier);
    const bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
    const bool alt = event->modifiers().testFlag(Qt::AltModifier);

    if (!windowSettings_.allowReloading &&
        (event->key() == Qt::Key_F5 || (ctrl && event->key() == Qt::Key_R))) {
        event->accept();
        return;
    }

    if (!windowSettings_.allowBackwardNavigation &&
        ((alt && event->key() == Qt::Key_Left) || event->key() == Qt::Key_Backspace)) {
        event->accept();
        return;
    }

    if (!windowSettings_.allowForwardNavigation && (alt && event->key() == Qt::Key_Right)) {
        event->accept();
        return;
    }

    if (!session_.settings().browser.allowPrint && ctrl && event->key() == Qt::Key_P) {
        event->accept();
        return;
    }

    if (!session_.settings().browser.allowFind && ctrl && event->key() == Qt::Key_F) {
        event->accept();
        return;
    }

    if (!session_.settings().browser.allowPageZoom &&
        ctrl &&
        (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal || event->key() == Qt::Key_Minus ||
         event->key() == Qt::Key_0)) {
        event->accept();
        return;
    }

    if (!windowSettings_.allowDeveloperConsole && event->key() == Qt::Key_F12) {
        event->accept();
        return;
    }

    if (ctrl && shift && event->key() == Qt::Key_Q) {
        if (isMainWindow_ &&
            session_.requestApplicationQuit(
                this,
                QStringLiteral("Enter the quit password to close this exam session."))) {
            QApplication::quit();
        }
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void BrowserWindow::focusInEvent(QFocusEvent *event)
{
    QMainWindow::focusInEvent(event);
    notifyTaskbarStateChanged();
}

void BrowserWindow::applyWindowFlags()
{
    setWindowFlag(Qt::WindowStaysOnTopHint, windowSettings_.alwaysOnTop);
    setWindowFlag(Qt::FramelessWindowHint, windowSettings_.frameless);
    if (!windowSettings_.allowMinimize) {
        setWindowFlag(Qt::WindowMinimizeButtonHint, false);
    }
}

void BrowserWindow::applyWindowGeometry()
{
    const QScreen *screen = QGuiApplication::primaryScreen();
    const QRect available = screen ? screen->availableGeometry() : QRect(100, 100, 1280, 800);
    const QRect screenGeometry = screen ? screen->geometry() : available;

    if (windowSettings_.fullScreenMode) {
        resize(available.size());
        showFullScreen();
        return;
    }

    if (windowSettings_.frameless && windowSettings_.alwaysOnTop) {
        setGeometry(screenGeometry);
        showMaximized();
        return;
    }

    int width = available.width();
    int height = available.height();

    if (windowSettings_.absoluteWidth > 0) {
        width = windowSettings_.absoluteWidth;
    } else if (windowSettings_.relativeWidth > 0) {
        width = (available.width() * windowSettings_.relativeWidth) / 100;
    }

    if (windowSettings_.absoluteHeight > 0) {
        height = windowSettings_.absoluteHeight;
    } else if (windowSettings_.relativeHeight > 0) {
        height = (available.height() * windowSettings_.relativeHeight) / 100;
    }

    width = qMin(width, available.width());
    height = qMin(height, available.height());
    resize(width, height);

    int x = available.x();
    switch (windowSettings_.position) {
    case seb::WindowPosition::Left:
        x = available.left();
        break;
    case seb::WindowPosition::Center:
        x = available.left() + (available.width() - width) / 2;
        break;
    case seb::WindowPosition::Right:
        x = available.right() - width + 1;
        break;
    }

    const int y = available.top() + (available.height() - height) / 2;
    move(x, y);
}

void BrowserWindow::configureToolbar()
{
    const bool showToolbar =
        windowSettings_.showToolbar ||
        windowSettings_.allowAddressBar ||
        windowSettings_.showHomeButton ||
        windowSettings_.showReloadButton;

    if (!showToolbar) {
        return;
    }

    toolbar_ = addToolBar(QStringLiteral("Browser"));
    toolbar_->setMovable(false);

    auto *backAction = toolbar_->addAction(QStringLiteral("Back"));
    backAction->setEnabled(windowSettings_.allowBackwardNavigation);
    connect(backAction, &QAction::triggered, this, [this] {
        if (windowSettings_.allowBackwardNavigation) {
            view_->back();
        }
    });

    auto *forwardAction = toolbar_->addAction(QStringLiteral("Forward"));
    forwardAction->setEnabled(windowSettings_.allowForwardNavigation);
    connect(forwardAction, &QAction::triggered, this, [this] {
        if (windowSettings_.allowForwardNavigation) {
            view_->forward();
        }
    });

    if (windowSettings_.showReloadButton) {
        auto *reloadAction = toolbar_->addAction(QStringLiteral("Reload"));
        reloadAction->setEnabled(windowSettings_.allowReloading);
        connect(reloadAction, &QAction::triggered, this, &BrowserWindow::reloadPage);
    }

    if (windowSettings_.showHomeButton && session_.homeUrl().isValid()) {
        auto *homeAction = toolbar_->addAction(QStringLiteral("Home"));
        connect(homeAction, &QAction::triggered, this, &BrowserWindow::navigateHome);
    }

    if (windowSettings_.allowAddressBar) {
        addressBar_ = new QLineEdit(toolbar_);
        addressBar_->setClearButtonEnabled(true);
        addressBar_->setPlaceholderText(QStringLiteral("Enter exam URL"));
        toolbar_->addWidget(addressBar_);
        connect(addressBar_, &QLineEdit::returnPressed, this, [this] {
            view_->setUrl(QUrl::fromUserInput(addressBar_->text().trimmed()));
        });
    }
}

void BrowserWindow::configureShortcuts()
{
    auto *reloadAction = new QAction(this);
    reloadAction->setShortcut(QKeySequence::Refresh);
    addAction(reloadAction);
    connect(reloadAction, &QAction::triggered, this, &BrowserWindow::reloadPage);

    auto *reloadAction2 = new QAction(this);
    reloadAction2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    addAction(reloadAction2);
    connect(reloadAction2, &QAction::triggered, this, &BrowserWindow::reloadPage);

    auto *backAction = new QAction(this);
    backAction->setShortcut(QKeySequence::Back);
    addAction(backAction);
    connect(backAction, &QAction::triggered, this, [this] {
        if (windowSettings_.allowBackwardNavigation) {
            view_->back();
        }
    });

    auto *forwardAction = new QAction(this);
    forwardAction->setShortcut(QKeySequence::Forward);
    addAction(forwardAction);
    connect(forwardAction, &QAction::triggered, this, [this] {
        if (windowSettings_.allowForwardNavigation) {
            view_->forward();
        }
    });

    auto *findAction = new QAction(this);
    findAction->setShortcut(QKeySequence::Find);
    addAction(findAction);
    connect(findAction, &QAction::triggered, this, &BrowserWindow::findInPage);

    auto *devToolsAction = new QAction(this);
    devToolsAction->setShortcut(QKeySequence(Qt::Key_F12));
    addAction(devToolsAction);
    connect(devToolsAction, &QAction::triggered, this, &BrowserWindow::openDevTools);

    if (windowSettings_.allowAddressBar && addressBar_) {
        auto *focusAddressBar = new QAction(this);
        focusAddressBar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
        addAction(focusAddressBar);
        connect(focusAddressBar, &QAction::triggered, this, [this] {
            addressBar_->setFocus();
            addressBar_->selectAll();
        });
    }
}

void BrowserWindow::findInPage()
{
    if (!session_.settings().browser.allowFind) {
        return;
    }

    bool accepted = false;
    const QString text = QInputDialog::getText(
        this,
        QStringLiteral("Find in Page"),
        QStringLiteral("Search text:"),
        QLineEdit::Normal,
        QString(),
        &accepted);
    if (!accepted || text.isEmpty()) {
        return;
    }

    page_->findText(QString());
    page_->findText(text);
}

void BrowserWindow::navigateHome()
{
    const QUrl home = session_.homeUrl();
    if (!home.isValid()) {
        return;
    }

    if (!session_.promptForHomeNavigation(this)) {
        return;
    }

    view_->setUrl(home);
}

void BrowserWindow::openDevTools()
{
    if (!windowSettings_.allowDeveloperConsole) {
        return;
    }

    auto *devTools = new QWebEngineView();
    auto *devPage = new QWebEnginePage(session_.profile(), devTools);
    page_->setDevToolsPage(devPage);
    devTools->setPage(devPage);
    devTools->setAttribute(Qt::WA_DeleteOnClose);
    devTools->resize(1100, 700);
    devTools->show();
}

void BrowserWindow::handleNewWindowRequest(QWebEngineNewWindowRequest &request)
{
    bool openInSameWindow = false;
    const QUrl target = request.requestedUrl();
    const QUrl opener = view_->url();

    const QString scheme = target.scheme().toLower();
    if (scheme == QStringLiteral("seb") || scheme == QStringLiteral("sebs") ||
        target.path().endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive)) {
        session_.openSebResource(target, this);
        return;
    }

    if (!session_.isPopupAllowed(opener, target, &openInSameWindow)) {
        return;
    }

    if (openInSameWindow) {
        request.openIn(page_);
        return;
    }

    auto *window = session_.createWindow(target, false);
    window->show();
    request.openIn(window->page());
}

void BrowserWindow::reloadPage()
{
    if (!windowSettings_.allowReloading) {
        return;
    }

    if (windowSettings_.showReloadWarning && !session_.confirmReload(this)) {
        return;
    }

    view_->reload();
}

void BrowserWindow::updateAddressBar(const QUrl &url)
{
    if (addressBar_) {
        addressBar_->setText(url.toString());
    }
    if (taskbar_) {
        taskbar_->setCurrentUrl(url);
    }
    notifyTaskbarStateChanged();
}

void BrowserWindow::notifyTaskbarStateChanged()
{
    session_.notifyBrowserWindowStateChanged(this);
}
