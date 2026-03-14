#include "network_resource_loader.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace seb::configuration::data_resources {

NetworkResourceLoader::NetworkResourceLoader(const contracts::AppConfig &appConfig)
    : appConfig_(appConfig)
{
}

bool NetworkResourceLoader::canLoad(const QUrl &resource) const
{
    return resource.scheme().startsWith(QStringLiteral("http")) ||
           resource.scheme() == appConfig_.sebUriScheme ||
           resource.scheme() == appConfig_.sebUriSchemeSecure;
}

contracts::LoadStatus NetworkResourceLoader::tryLoad(const QUrl &resource, QByteArray &data) const
{
    QNetworkAccessManager manager;
    QNetworkRequest request(resource);
    auto *reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return contracts::LoadStatus::LoadWithBrowser;
    }

    data = reply->readAll();
    reply->deleteLater();
    return contracts::LoadStatus::Success;
}

}  // namespace seb::configuration::data_resources
