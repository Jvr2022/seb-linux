#include "qt_webengine_view.h"

#if SEB_HAS_QTWEBENGINE
#include "qt_webengine_profile.h"
#include "browser/Clipboard.h"
#include "configuration/contracts/app_config.h"
#include "configuration/contracts/cryptography/i_key_generator.h"
#include "settings/Browser/BrowserSettings.h"
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QVariantMap>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineHistory>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QAuthenticator>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWebEngineSettings>
#include <QPrintDialog>
#include <QPrinter>
#include <QPointer>
#include <optional>

namespace seb::browser {

namespace {

class SebSecurityBridge final : public QObject
{
    Q_OBJECT

public:
    SebSecurityBridge(
        seb::configuration::contracts::cryptography::IKeyGenerator *keyGenerator,
        const seb::settings::browser::BrowserSettings &settings,
        QObject *parent)
        : QObject(parent)
        , keyGenerator_(keyGenerator)
        , settings_(settings)
    {
    }

    Q_INVOKABLE QVariantMap keysForUrl(const QString &url) const
    {
        QVariantMap keys;
        if (!keyGenerator_) {
            return keys;
        }

        keys.insert(
            QStringLiteral("browserExamKey"),
            keyGenerator_->calculateBrowserExamKeyHash(
                settings_.ConfigurationKey,
                settings_.BrowserExamKeySalt,
                url));
        keys.insert(
            QStringLiteral("configKey"),
            keyGenerator_->calculateConfigurationKeyHash(
                settings_.ConfigurationKey,
                url));
        return keys;
    }

private:
    seb::configuration::contracts::cryptography::IKeyGenerator *keyGenerator_ = nullptr;
    seb::settings::browser::BrowserSettings settings_;
};

class SebClipboardBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)

public:
    SebClipboardBridge(seb::browser::Clipboard *clipboard, QObject *parent)
        : QObject(parent), clipboard_(clipboard)
    {
        connect(clipboard, &seb::browser::Clipboard::changed, this, [this](const QString &, const QString &source) {
            emit contentChanged();
            emit updated(source, content());
        });
    }

    QString content() const
    {
        return clipboard_ ? QString::fromLatin1(clipboard_->content().toUtf8().toBase64()) : QString();
    }

    Q_INVOKABLE void update(const QString &source, const QString &encoded)
    {
        if (!clipboard_ || source.size() > 128 || encoded.size() > 8 * 1024 * 1024) return;
        const auto decoded = QByteArray::fromBase64Encoding(encoded.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if (decoded.decodingStatus == QByteArray::Base64DecodingStatus::Ok)
            clipboard_->update(QString::fromUtf8(decoded.decoded), source);
    }

signals:
    void commandRequested(const QString &command);
    void contentChanged();
    void updated(const QString &source, const QString &encoded);

private:
    QPointer<seb::browser::Clipboard> clipboard_;
};

QString javaScriptString(const QString &value)
{
    QJsonArray array;
    array.append(value);
    const QByteArray encoded =
        QJsonDocument(array).toJson(QJsonDocument::Compact);
    return QString::fromUtf8(encoded.mid(1, encoded.size() - 2));
}

}  // namespace

class InterceptingPage : public QWebEnginePage
{
public:
    InterceptingPage(QWebEngineProfile *profile, QtWebEngineWebView *wrapper, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent), wrapper_(wrapper) {}

protected:
    QStringList chooseFiles(
        FileSelectionMode mode,
        const QStringList &oldFiles,
        const QStringList &acceptedMimeTypes) override
    {
        if (!wrapper_->fileSelectionDelegate()) {
            return {};
        }
        return wrapper_->fileSelectionDelegate()(
            static_cast<int>(mode), oldFiles, acceptedMimeTypes);
    }

    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override
    {
        if (wrapper_->delegate() && !wrapper_->delegate()(url, isMainFrame)) {
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }
    
private:
    QtWebEngineWebView *wrapper_;
};

QtWebEngineWebView::QtWebEngineWebView(QtWebEngineProfile *profile, QWidget *parent)
    : IWebView(parent)
{
    view_ = new QWebEngineView(parent);
    view_->setAcceptDrops(false);
    view_->setContextMenuPolicy(Qt::NoContextMenu);
    qApp->installEventFilter(this);
    auto *page = new InterceptingPage(profile->nativeProfile(), this, view_);
    view_->setPage(page);



    connect(view_, &QWebEngineView::urlChanged, this, &QtWebEngineWebView::urlChanged);
    connect(view_, &QWebEngineView::titleChanged, this, &QtWebEngineWebView::titleChanged);
    
    connect(page, &QWebEnginePage::newWindowRequested, this, [this](auto &request) {
        if (request.destination() == QWebEngineNewWindowRequest::InNewBackgroundTab) return;
        contracts::IWebView *target = nullptr;
        emit newWindowRequested(request.requestedUrl(), target);
        auto *nativeTarget = qobject_cast<QtWebEngineWebView *>(target);
        if (nativeTarget) request.openIn(nativeTarget->view_->page());
    });
    connect(page, &QWebEnginePage::windowCloseRequested, this, &QtWebEngineWebView::closeRequested);
    connect(page, &QWebEnginePage::printRequested, this, [this] {
        if (configured_ && settings_.AllowPrint) emit commandRequested(Command::Print);
    });
    connect(view_, &QWebEngineView::printFinished, this, [this](bool) { printer_.reset(); printing_ = false; });

    connect(page, &QWebEnginePage::proxyAuthenticationRequired,
            this, &QtWebEngineWebView::proxyAuthenticationRequired);
}

QtWebEngineWebView::~QtWebEngineWebView()
{
    qApp->removeEventFilter(this);
    delete view_;
}

QWidget *QtWebEngineWebView::widget() const
{
    return view_;
}

QUrl QtWebEngineWebView::url() const
{
    return view_->url();
}

void QtWebEngineWebView::setUrl(const QUrl &url)
{
    view_->setUrl(url);
}

void QtWebEngineWebView::download(const QUrl &url, const QString &suggestedFileName)
{
    view_->page()->download(url, suggestedFileName);
}

void QtWebEngineWebView::configureBrowser(
    const seb::settings::AppSettings &configuration,
    const seb::settings::browser::WindowSettings &window)
{
    const auto &settings = configuration.Browser;
    settings_ = settings;
    clipboardPolicy_ = configuration.Security.ClipboardPolicy;
    keyboardSettings_ = configuration.Keyboard;
    mouseSettings_ = configuration.Mouse;
    windowSettings_ = window;
    configured_ = true;
    auto *native = view_->settings();
    native->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows,
                         settings.PopupPolicy != seb::settings::browser::PopupPolicy::Block);
    native->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, clipboardPolicy_ == seb::settings::security::ClipboardPolicy::Allow);
    native->setAttribute(QWebEngineSettings::JavascriptCanPaste, clipboardPolicy_ == seb::settings::security::ClipboardPolicy::Allow);
    native->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    native->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    native->setUnknownUrlSchemePolicy(QWebEngineSettings::DisallowUnknownUrlSchemes);
    if (clipboardPolicy_ == seb::settings::security::ClipboardPolicy::Block) {
        QWebEngineScript script;
        script.setName(QStringLiteral("seb-block-clipboard"));
        script.setSourceCode(QStringLiteral(
            "['copy','cut','paste'].forEach(function(type){window.addEventListener(type,function(event){"
            "event.preventDefault();event.stopImmediatePropagation();},true);});"));
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setWorldId(QWebEngineScript::ApplicationWorld);
        script.setRunsOnSubFrames(true);
        view_->page()->scripts().insert(script);
    }
    if (!settings.AllowPageZoom) {
        QFile source(QStringLiteral(":/browser/content/PageZoom.js"));
        if (source.open(QIODevice::ReadOnly)) {
            QWebEngineScript script;
            script.setName(QStringLiteral("seb-wheel-zoom-policy"));
            script.setSourceCode(QString::fromUtf8(source.readAll()));
            script.setInjectionPoint(QWebEngineScript::DocumentCreation);
            script.setWorldId(QWebEngineScript::ApplicationWorld);
            script.setRunsOnSubFrames(true);
            view_->page()->scripts().insert(script);
        }
    }
}

void QtWebEngineWebView::initializeClipboard(seb::browser::Clipboard *clipboard)
{
    clipboard_ = clipboard;
}

void QtWebEngineWebView::setZoomFactor(double factor)
{
    if (configured_ && settings_.AllowPageZoom) view_->setZoomFactor(qBound(0.25, factor, 3.0));
}

void QtWebEngineWebView::print()
{
    if (!configured_ || !settings_.AllowPrint || printing_) return;
    printing_ = true;
    auto printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    QPrintDialog dialog(printer.get(), view_);
    if (dialog.exec() == QDialog::Accepted) {
        printer_ = std::move(printer);
        view_->print(printer_.get());
    } else printing_ = false;
}

bool QtWebEngineWebView::eventFilter(QObject *watched, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(watched);
    if (!event || !widget || widget->window() != view_->window() ||
        (widget != view_ && !view_->isAncestorOf(widget)))
        return QObject::eventFilter(watched, event);
    if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove || event->type() == QEvent::Drop) {
        event->ignore();
        return true;
    }
    if (event->type() == QEvent::ContextMenu) return true;
    if (event->type() == QEvent::NativeGesture && configured_ && !settings_.AllowPageZoom) return true;
    if (event->type() == QEvent::Wheel) {
        auto *wheel = static_cast<QWheelEvent *>(event);
        if (wheel->modifiers().testFlag(Qt::ControlModifier)) {
            if (wheel->angleDelta().y() != 0)
                emit commandRequested(wheel->angleDelta().y() > 0 ? Command::ZoomIn : Command::ZoomOut);
            event->accept();
            return true;
        }
    }
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (configured_ && ((mouse->button() == Qt::MiddleButton &&
             (!mouseSettings_.AllowMiddleButton || clipboardPolicy_ != seb::settings::security::ClipboardPolicy::Allow)) ||
             (mouse->button() == Qt::RightButton && !mouseSettings_.AllowRightButton))) {
            event->accept();
            return true;
        }
        if (mouse->button() == Qt::BackButton || mouse->button() == Qt::ForwardButton) {
            if (event->type() == QEvent::MouseButtonRelease)
                emit commandRequested(mouse->button() == Qt::BackButton ? Command::Back : Command::Forward);
            event->accept();
            return true;
        }
    }
    if (event->type() != QEvent::ShortcutOverride && event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return QObject::eventFilter(watched, event);
    auto *key = static_cast<QKeyEvent *>(event);
    const auto modifiers = key->modifiers();
    const bool ctrl = modifiers.testFlag(Qt::ControlModifier);
    const bool alt = modifiers.testFlag(Qt::AltModifier);
    const bool shift = modifiers.testFlag(Qt::ShiftModifier);
    if (configured_) {
        const bool functionKeys[] = {keyboardSettings_.AllowF1, keyboardSettings_.AllowF2, keyboardSettings_.AllowF3,
            keyboardSettings_.AllowF4, keyboardSettings_.AllowF5, keyboardSettings_.AllowF6, keyboardSettings_.AllowF7,
            keyboardSettings_.AllowF8, keyboardSettings_.AllowF9, keyboardSettings_.AllowF10, keyboardSettings_.AllowF11,
            keyboardSettings_.AllowF12};
        const bool blocked = (key->key() >= Qt::Key_F1 && key->key() <= Qt::Key_F12 && !functionKeys[key->key() - Qt::Key_F1]) ||
            (key->key() == Qt::Key_Escape && (!keyboardSettings_.AllowEsc || (alt && !keyboardSettings_.AllowAltEsc) || (ctrl && !keyboardSettings_.AllowCtrlEsc))) ||
            (alt && key->key() == Qt::Key_F4 && !keyboardSettings_.AllowAltF4) ||
            (alt && key->key() == Qt::Key_Tab && !keyboardSettings_.AllowAltTab) ||
            (key->key() == Qt::Key_Print && !keyboardSettings_.AllowPrintScreen) ||
            ((key->key() == Qt::Key_Meta || key->key() == Qt::Key_Super_L || key->key() == Qt::Key_Super_R) && !keyboardSettings_.AllowSystemKey);
        if (blocked) { event->accept(); return true; }
    }
    if (configured_ && !alt) {
        QString clipboardCommand;
        if ((ctrl && key->key() == Qt::Key_C) || (ctrl && key->key() == Qt::Key_Insert)) clipboardCommand = QStringLiteral("copy");
        else if ((ctrl && key->key() == Qt::Key_X) || (shift && key->key() == Qt::Key_Delete)) clipboardCommand = QStringLiteral("cut");
        else if ((ctrl && key->key() == Qt::Key_V) || (shift && key->key() == Qt::Key_Insert)) clipboardCommand = QStringLiteral("paste");
        if (!clipboardCommand.isEmpty()) {
            event->accept();
            const bool keyAllowed = clipboardCommand == QStringLiteral("copy") ? keyboardSettings_.AllowCtrlC :
                clipboardCommand == QStringLiteral("cut") ? keyboardSettings_.AllowCtrlX : keyboardSettings_.AllowCtrlV;
            if (!keyAllowed || clipboardPolicy_ == seb::settings::security::ClipboardPolicy::Block) return true;
            if (clipboardPolicy_ == seb::settings::security::ClipboardPolicy::Isolated) {
                if (clipboardCommand_ && event->type() == QEvent::KeyPress && !key->isAutoRepeat()) clipboardCommand_(clipboardCommand);
                return true;
            }
            event->ignore();
        }
    }
    std::optional<Command> command;
    if (key->key() == Qt::Key_F5 || (ctrl && !alt && key->key() == Qt::Key_R)) command = Command::Reload;
    else if (alt && key->key() == Qt::Key_Left) command = Command::Back;
    else if (alt && key->key() == Qt::Key_Right) command = Command::Forward;
    else if (alt && key->key() == Qt::Key_Home) command = Command::Home;
    else if (ctrl && !alt && key->key() == Qt::Key_F) command = Command::Find;
    else if (ctrl && !alt && key->key() == Qt::Key_L) command = Command::FocusAddressBar;
    else if (ctrl && !alt && key->key() == Qt::Key_P) command = Command::Print;
    else if (ctrl && !alt && (key->key() == Qt::Key_Plus || key->key() == Qt::Key_Equal)) command = Command::ZoomIn;
    else if (ctrl && !alt && key->key() == Qt::Key_Minus) command = Command::ZoomOut;
    else if (ctrl && !alt && key->key() == Qt::Key_0) command = Command::ZoomReset;
    else if (key->key() == Qt::Key_F12 || (ctrl && shift && (key->key() == Qt::Key_I || key->key() == Qt::Key_J))) command = Command::DeveloperTools;
    else if (ctrl && shift && key->key() == Qt::Key_Q) command = Command::Quit;
    if (command) {
        event->accept();
        if (event->type() == QEvent::KeyPress && !key->isAutoRepeat()) emit commandRequested(*command);
        return true;
    }
    if (key->key() == Qt::Key_F11 || (ctrl && (key->key() == Qt::Key_U || key->key() == Qt::Key_S || key->key() == Qt::Key_O))) {
        event->accept();
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void QtWebEngineWebView::back()
{
    if (configured_ && windowSettings_.AllowBackwardNavigation) view_->back();
}

void QtWebEngineWebView::forward()
{
    if (configured_ && windowSettings_.AllowForwardNavigation) view_->forward();
}

void QtWebEngineWebView::reload()
{
    if (configured_ && windowSettings_.AllowReloading) view_->reload();
}

void QtWebEngineWebView::openDevTools()
{
    if (!configured_ || !windowSettings_.AllowDeveloperConsole) return;
    auto *devTools = new QWebEngineView(view_->window());
    devTools->setWindowFlag(Qt::Window);
    auto *devPage = new QWebEnginePage(view_->page()->profile(), devTools);
    view_->page()->setDevToolsPage(devPage);
    devTools->setPage(devPage);
    devTools->setAttribute(Qt::WA_DeleteOnClose);
    devTools->resize(1100, 700);
    devTools->show();
}

bool QtWebEngineWebView::canGoBack() const
{
    return view_->history()->canGoBack();
}

bool QtWebEngineWebView::canGoForward() const
{
    return view_->history()->canGoForward();
}

void QtWebEngineWebView::findText(const QString &text)
{
    if (!configured_ || !settings_.AllowFind) return;
    view_->page()->findText(QString());
    view_->page()->findText(text);
}

void QtWebEngineWebView::setNavigationRequestDelegate(contracts::IWebView::NavigationRequestDelegate delegate)
{
    navigationDelegate_ = std::move(delegate);
}

void QtWebEngineWebView::setFileSelectionDelegate(contracts::IWebView::FileSelectionDelegate delegate)
{
    fileSelectionDelegate_ = std::move(delegate);
}

void QtWebEngineWebView::initializeSebApi(
    seb::configuration::contracts::cryptography::IKeyGenerator *keyGenerator,
    const seb::settings::browser::BrowserSettings &settings,
    const seb::configuration::contracts::AppConfig &appConfig)
{
    if (sebApiInitialized_ || !keyGenerator || !view_ || !view_->page()) {
        return;
    }

    auto *page = view_->page();
    webChannel_ = new QWebChannel(page);
    webChannel_->registerObject(
        QStringLiteral("sebSecurityBridge"),
        new SebSecurityBridge(keyGenerator, settings, webChannel_));
    if (settings.UseIsolatedClipboard && clipboard_) {
        auto *clipboardBridge = new SebClipboardBridge(clipboard_, webChannel_);
        webChannel_->registerObject(QStringLiteral("sebClipboardBridge"), clipboardBridge);
        clipboardCommand_ = [bridge = QPointer<SebClipboardBridge>(clipboardBridge)](const QString &command) {
            if (bridge) emit bridge->commandRequested(command);
        };
        QFile clipboardSource(QStringLiteral(":/browser/content/Clipboard.js"));
        if (clipboardSource.open(QIODevice::ReadOnly)) {
            QWebEngineScript clipboardScript;
            clipboardScript.setName(QStringLiteral("seb-isolated-clipboard"));
            clipboardScript.setSourceCode(QStringLiteral(
                "(function(){ var bridge=null; var pending=null;"
                "const SafeExamBrowser={};"
                "const CefSharp={PostMessage:function(message){"
                "if(bridge) bridge.update(message.Id,message.Content); else pending=message; }};\n") +
                QString::fromUtf8(clipboardSource.readAll()) + QStringLiteral(
                "\nwindow.__sebInstallClipboard=function(native){"
                "if(!native || bridge) return; bridge=native;"
                "bridge.updated.connect(function(id,encoded){SafeExamBrowser.clipboard.update(id,encoded);});"
                "bridge.commandRequested.connect(function(command){"
                "var target=document.activeElement;"
                "if(!document.hasFocus() || !target || target.tagName==='IFRAME' || target.tagName==='FRAME') return;"
                "var event={isTrusted:true,target:target,preventDefault:function(){},stopImmediatePropagation:function(){}};"
                "if(command==='copy') onCopy(event); else if(command==='cut') onCut(event); else if(command==='paste') onPaste(event);"
                "});"
                "if(pending){bridge.update(pending.Id,pending.Content); pending=null;}"
                "else SafeExamBrowser.clipboard.update('',bridge.content);"
                "}; document.dispatchEvent(new Event('seb-clipboard-ready')); })();"));
            clipboardScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
            clipboardScript.setWorldId(QWebEngineScript::ApplicationWorld);
            clipboardScript.setRunsOnSubFrames(true);
            page->scripts().insert(clipboardScript);
        }
    }
    page->setWebChannel(webChannel_, QWebEngineScript::ApplicationWorld);

    QFile channelSource(QStringLiteral(":/browser/content/qwebchannel.js"));
    QFile bridgeSource(QStringLiteral(":/browser/content/ApiBridge.js"));
    QFile apiSource(QStringLiteral(":/browser/content/Api.js"));
    if (!channelSource.open(QIODevice::ReadOnly) ||
        !bridgeSource.open(QIODevice::ReadOnly) ||
        !apiSource.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to load the Qt WebChannel JavaScript bridge.";
        return;
    }

    QWebEngineScriptCollection &scripts = page->scripts();
    const auto removeScript = [&scripts](const QString &name) {
        const auto existing = scripts.find(name);
        for (const auto &script : existing) {
            scripts.remove(script);
        }
    };
    removeScript(QStringLiteral("seb-qwebchannel"));
    removeScript(QStringLiteral("seb-security-bridge"));
    removeScript(QStringLiteral("seb-security-api"));

    QWebEngineScript bridgeScript;
    bridgeScript.setName(QStringLiteral("seb-security-bridge"));
    bridgeScript.setSourceCode(
        QString::fromUtf8(channelSource.readAll()) +
        QLatin1Char('\n') +
        QString::fromUtf8(bridgeSource.readAll()));
    bridgeScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    bridgeScript.setWorldId(QWebEngineScript::ApplicationWorld);
    bridgeScript.setRunsOnSubFrames(true);
    scripts.insert(bridgeScript);

    QWebEngineScript apiScript;
    apiScript.setName(QStringLiteral("seb-security-api"));
    const QString version =
        QStringLiteral("SEB_Linux_") + appConfig.programBuildVersion;
    apiScript.setSourceCode(
        QStringLiteral("window.__sebVersion = %1;\n")
            .arg(javaScriptString(version)) +
        QString::fromUtf8(apiSource.readAll()));
    apiScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    apiScript.setWorldId(QWebEngineScript::MainWorld);
    apiScript.setRunsOnSubFrames(true);
    scripts.insert(apiScript);
    sebApiInitialized_ = true;
}

}  // namespace seb::browser
#endif

#if SEB_HAS_QTWEBENGINE
#include "qt_webengine_view.moc"
#endif
