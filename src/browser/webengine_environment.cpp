#include "webengine_environment.h"
#include "webengine_compat.h"
#include <QProcess>
#include <QSet>
#include <QUrl>
#include <QRegularExpression>

namespace seb::browser {
namespace {
bool initialized = false;
QStringList configuredProxyArguments;

bool failure(QString *error, const QString &message)
{
    if (error) *error = message;
    return false;
}

bool proxyArguments(const seb::settings::browser::ProxySettings &settings, QStringList &arguments, QString *error)
{
    using Protocol = seb::settings::browser::proxy::ProxyProtocol;
    if (settings.Policy != seb::settings::browser::proxy::ProxyPolicy::Custom) return true;
    if (settings.AutoConfigure) {
        const QUrl url(settings.AutoConfigureUrl, QUrl::StrictMode);
        if (!url.isValid() || (url.scheme() != QStringLiteral("https") && url.scheme() != QStringLiteral("http")) ||
            url.host().isEmpty() || !url.userInfo().isEmpty() || url.hasFragment())
            return failure(error, QStringLiteral("Invalid proxy auto-configuration URL."));
        arguments.append(QStringLiteral("--proxy-pac-url=") + url.toString(QUrl::FullyEncoded));
    }
    if (settings.AutoDetect) arguments.append(QStringLiteral("--proxy-auto-detect"));
    QStringList servers;
    QSet<int> protocols;
    for (const auto &proxy : settings.Proxies) {
        QString host = proxy.Host.trimmed();
        if (host.startsWith(QLatin1Char('[')) && host.endsWith(QLatin1Char(']'))) host = host.mid(1, host.size() - 2);
        QUrl address;
        address.setScheme(QStringLiteral("http"));
        address.setHost(host);
        address.setPort(proxy.Port);
        if (host.isEmpty() || proxy.Port < 1 || proxy.Port > 65535 || !address.isValid() ||
            host.contains(QRegularExpression(QStringLiteral("[\\s/@;?#=]"))))
            return failure(error, QStringLiteral("Invalid exam proxy host or port."));
        if (protocols.contains(int(proxy.Protocol))) return failure(error, QStringLiteral("Multiple exam proxies for one protocol are ambiguous."));
        protocols.insert(int(proxy.Protocol));
        const QString authority = address.authority(QUrl::FullyEncoded);
        QString scheme;
        switch (proxy.Protocol) {
        case Protocol::Http: scheme = QStringLiteral("http"); break;
        case Protocol::Https: scheme = QStringLiteral("https"); break;
        case Protocol::Ftp: scheme = QStringLiteral("ftp"); break;
        case Protocol::Socks: scheme = QStringLiteral("socks"); break;
        }
        servers.append(scheme + QLatin1Char('=') + (proxy.Protocol == Protocol::Socks ? QStringLiteral("socks5://") : QString()) + authority);
    }
    if (!servers.isEmpty()) arguments.append(QStringLiteral("--proxy-server=") + servers.join(QLatin1Char(';')));
    if (!settings.BypassList.isEmpty()) arguments.append(QStringLiteral("--proxy-bypass-list=") + settings.BypassList.join(QLatin1Char(';')));
    if (!settings.AutoConfigure && !settings.AutoDetect && servers.isEmpty()) arguments.append(QStringLiteral("--no-proxy-server"));
    return true;
}
}

bool applyWebEngineEnvironment(const seb::settings::browser::BrowserSettings &settings, QString *error)
{
    QStringList arguments;
    if (!proxyArguments(settings.Proxy, arguments, error)) return false;
#if !SEB_HAS_QTWEBENGINE
    if (!arguments.isEmpty()) return failure(error, QStringLiteral("Custom exam proxy routing is unavailable in the WebKitGTK compatibility backend."));
    return true;
#else
    if (initialized && configuredProxyArguments != arguments)
        return failure(error, QStringLiteral("This configuration changes Chromium proxy routing. Restart Safe Exam Browser with this configuration to apply it."));
    if (initialized) return true;
    QStringList flags = QProcess::splitCommand(qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS"));
    const QStringList managed{QStringLiteral("--proxy-auto-detect"), QStringLiteral("--proxy-pac-url"),
        QStringLiteral("--proxy-server"), QStringLiteral("--proxy-bypass-list"), QStringLiteral("--no-proxy-server")};
    for (qsizetype i = 0; i < flags.size();) {
        const QString name = flags.at(i).section(QLatin1Char('='), 0, 0);
        if (!managed.contains(name)) { ++i; continue; }
        const bool separateValue = !flags.at(i).contains(QLatin1Char('=')) &&
            name != QStringLiteral("--proxy-auto-detect") && name != QStringLiteral("--no-proxy-server");
        flags.removeAt(i);
        if (separateValue && i < flags.size() && !flags.at(i).startsWith(QStringLiteral("--"))) flags.removeAt(i);
    }
    configuredProxyArguments = arguments;
    flags.append(arguments);
    for (auto &flag : flags) if (flag.contains(QLatin1Char(' '))) flag = QLatin1Char('"') + flag.replace(QStringLiteral("\""), QStringLiteral("\\\"")) + QLatin1Char('"');
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", flags.join(QLatin1Char(' ')).toUtf8());
    return true;
#endif
}

void freezeWebEngineEnvironment()
{
    initialized = true;
}
}
