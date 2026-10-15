#include "server_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ServerDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &server = settings.Server;
    if (key == keys::Server::Configuration) {
        const QVariantMap configuration = value.toMap();
        server.ApiEndpoint = string(configuration.value(keys::Server::ApiEndpoint));
        server.ClientName = string(configuration.value(keys::Server::ClientName));
        server.ClientSecret = string(configuration.value(keys::Server::ClientSecret));
        server.ExamId = string(configuration.value(keys::Server::ExamId));
        server.Institution = string(configuration.value(keys::Server::Institution));
        server.PingInterval = integer(configuration.value(keys::Server::PingInterval), server.PingInterval);
    }
    else if (key == keys::Server::FallbackPasswordHash) server.FallbackPasswordHash = string(value);
    else if (key == keys::Server::ForceRaiseHandMessage) server.Invigilation.ForceRaiseHandMessage = boolean(value, server.Invigilation.ForceRaiseHandMessage);
    else if (key == keys::Server::PerformFallback) server.PerformFallback = boolean(value, server.PerformFallback);
    else if (key == keys::Server::RequestAttempts) server.RequestAttempts = integer(value, server.RequestAttempts);
    else if (key == keys::Server::RequestAttemptInterval) server.RequestAttemptInterval = integer(value, server.RequestAttemptInterval);
    else if (key == keys::Server::RequestTimeout) server.RequestTimeout = integer(value, server.RequestTimeout);
    else if (key == keys::Server::ServerUrl) server.ServerUrl = string(value);
    else if (key == keys::Server::ShowRaiseHand) server.Invigilation.ShowRaiseHandNotification = boolean(value, server.Invigilation.ShowRaiseHandNotification);
}
}  // namespace seb::configuration::configuration_data::data_mapping
