#include "webengine_environment.h"

#include "webengine_compat.h"

#include <QStringList>

namespace seb::browser {
namespace {

QString buildProxyServerValue(const seb::ProxySettings &proxySettings)
{
    QStringList parts;
    for (const seb::ProxyConfiguration &proxy : proxySettings.proxies) {
        QString protocol;
        QString address = QStringLiteral("%1:%2").arg(proxy.host).arg(proxy.port);

        switch (proxy.protocol) {
        case seb::ProxyProtocol::Ftp:
            protocol = QStringLiteral("ftp");
            break;
        case seb::ProxyProtocol::Http:
            protocol = QStringLiteral("http");
            break;
        case seb::ProxyProtocol::Https:
            protocol = QStringLiteral("https");
            break;
        case seb::ProxyProtocol::Socks:
            protocol = QStringLiteral("socks5");
            address = QStringLiteral("socks5://%1").arg(address);
            break;
        }

        parts << QStringLiteral("%1=%2").arg(protocol, address);
    }

    return parts.join(';');
}

}  // namespace

void applyWebEngineEnvironment(const seb::SebSettings &settings)
{
#if !SEB_HAS_QTWEBENGINE
    Q_UNUSED(settings);
    return;
#else
    if (settings.browser.proxy.policy != seb::ProxyPolicy::Custom) {
        return;
    }

    QStringList flags = qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS").split(' ', Qt::SkipEmptyParts);

    if (settings.browser.proxy.autoDetect) {
        flags << QStringLiteral("--proxy-auto-detect");
    } else if (settings.browser.proxy.autoConfigure && !settings.browser.proxy.autoConfigureUrl.isEmpty()) {
        flags << QStringLiteral("--proxy-pac-url=%1").arg(settings.browser.proxy.autoConfigureUrl);
    } else {
        const QString proxyServer = buildProxyServerValue(settings.browser.proxy);
        if (!proxyServer.isEmpty()) {
            flags << QStringLiteral("--proxy-server=%1").arg(proxyServer);
        }
    }

    if (!settings.browser.proxy.bypassList.isEmpty()) {
        flags << QStringLiteral("--proxy-bypass-list=%1").arg(settings.browser.proxy.bypassList.join(';'));
    }

    flags.removeDuplicates();
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", flags.join(' ').toUtf8());
#endif
}

}  // namespace seb::browser
