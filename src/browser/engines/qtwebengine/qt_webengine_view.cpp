#include "qt_webengine_view.h"

#if SEB_HAS_QTWEBENGINE
#include "qt_webengine_profile.h"
#include "configuration/contracts/app_config.h"
#include "configuration/contracts/cryptography/i_key_generator.h"
#include "settings/Browser/BrowserSettings.h"
#include <QFile>
#include <QVariantMap>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineHistory>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QAuthenticator>

namespace seb::browser {

namespace {

class SebSecurityApi final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    SebSecurityApi(
        seb::configuration::contracts::cryptography::IKeyGenerator *keyGenerator,
        const seb::settings::browser::BrowserSettings &settings,
        const seb::configuration::contracts::AppConfig &appConfig,
        QObject *parent)
        : QObject(parent)
        , keyGenerator_(keyGenerator)
        , settings_(settings)
        , appConfig_(appConfig)
    {
    }

    QString version() const
    {
        return QStringLiteral("SEB_Linux_") + appConfig_.programBuildVersion;
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
    seb::configuration::contracts::AppConfig appConfig_;
};

}  // namespace

class InterceptingPage : public QWebEnginePage
{
public:
    InterceptingPage(QWebEngineProfile *profile, QtWebEngineWebView *wrapper, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent), wrapper_(wrapper) {}

protected:
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
    auto *page = new InterceptingPage(profile->nativeProfile(), this, view_);
    view_->setPage(page);

#if defined(QT_DEBUG) || defined(SEB_DEV_BYPASS_OPTION)
    if (profile->devBypass()) {
        QWebEngineScript script;
        script.setName(QStringLiteral("seb-stealth"));
        script.setSourceCode(QStringLiteral(
            "(function() {"
            "  const mask = (obj, prop, val) => {"
            "    try { Object.defineProperty(obj, prop, { get: () => val, enumerable: true, configurable: true }); } catch (e) {}"
            "  };"
            "  mask(navigator, 'webdriver', false);"
            "  mask(navigator, 'plugins', [1, 2, 3, 4, 5]);"
            "  mask(navigator, 'languages', ['en-US', 'en']);"
            "  window.chrome = { runtime: {} };"
            "  mask(document, 'hidden', false);"
            "  mask(document, 'visibilityState', 'visible');"
            "  window.addEventListener('blur', e => e.stopImmediatePropagation(), true);"
            "  document.addEventListener('blur', e => e.stopImmediatePropagation(), true);"
            "  window.addEventListener('mouseleave', e => e.stopImmediatePropagation(), true);"
            "})();"
        ));
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setRunsOnSubFrames(true);
        profile->nativeProfile()->scripts()->insert(script);
    }
#endif

    connect(view_, &QWebEngineView::urlChanged, this, &QtWebEngineWebView::urlChanged);
    connect(view_, &QWebEngineView::titleChanged, this, &QtWebEngineWebView::titleChanged);
    
    connect(page, &QWebEnginePage::newWindowRequested, this, [this](auto &request) {
        bool handled = false;
        emit this->newWindowRequested(request.requestedUrl(), handled);
        if (handled) {
            // In our simplified API we handle the window opening externally, so we drop the request object.
        }
    });

    connect(page, &QWebEnginePage::proxyAuthenticationRequired,
            this, &QtWebEngineWebView::proxyAuthenticationRequired);
}

QtWebEngineWebView::~QtWebEngineWebView()
{
    // delete view_; // Parent ownership will handle this usually, but view_ is a QWidget.
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

void QtWebEngineWebView::back()
{
    view_->back();
}

void QtWebEngineWebView::forward()
{
    view_->forward();
}

void QtWebEngineWebView::reload()
{
    view_->reload();
}

void QtWebEngineWebView::openDevTools()
{
    auto *devTools = new QWebEngineView();
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
    view_->page()->findText(QString());
    view_->page()->findText(text);
}

void QtWebEngineWebView::setNavigationRequestDelegate(contracts::IWebView::NavigationRequestDelegate delegate)
{
    navigationDelegate_ = std::move(delegate);
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
        QStringLiteral("sebSecurityApi"),
        new SebSecurityApi(keyGenerator, settings, appConfig, webChannel_));
    page->setWebChannel(webChannel_, QWebEngineScript::MainWorld);

    QFile channelSource(QStringLiteral(":/browser/content/qwebchannel.js"));
    QFile apiSource(QStringLiteral(":/browser/content/Api.js"));
    if (!channelSource.open(QIODevice::ReadOnly) || !apiSource.open(QIODevice::ReadOnly)) {
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
    removeScript(QStringLiteral("seb-security-api"));

    QWebEngineScript apiScript;
    apiScript.setName(QStringLiteral("seb-security-api"));
    apiScript.setSourceCode(
        QString::fromUtf8(channelSource.readAll()) +
        QLatin1Char('\n') +
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
