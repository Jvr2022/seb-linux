#include "server_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ServerDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &server = settings.Server;
    if (key == keys::Server::Configuration) server.Enabled = boolean(value, server.Enabled);
    else if (key == keys::Server::ApiEndpoint) server.ApiEndpoint = string(value);
    else if (key == keys::Server::ServerUrl) server.Url = string(value);
    else if (key == keys::Server::ClientName) server.ClientId = string(value);
    else if (key == keys::Server::ClientSecret) server.ClientSecret = string(value);
    else if (key == keys::Server::Institution) server.Institution = string(value);
    else if (key == keys::Server::FallbackPasswordHash) server.FallbackPasswordHash = string(value);
    else if (key == QStringLiteral("enableRemoteProctoring")) server.Invigilation.EnableRemoteProctoring = boolean(value, server.Invigilation.EnableRemoteProctoring);
    else if (key == keys::Server::ExamId) server.ExamId = string(value);
    else if (key == keys::Server::PerformFallback) server.PerformFallback = boolean(value, server.PerformFallback);
    else if (key == keys::Server::PingInterval) server.PingInterval = integer(value, server.PingInterval);
    else if (key == keys::Server::RequestAttempts) server.RequestAttempts = integer(value, server.RequestAttempts);
    else if (key == keys::Server::RequestAttemptInterval) server.RequestAttemptInterval = integer(value, server.RequestAttemptInterval);
    else if (key == keys::Server::RequestTimeout) server.RequestTimeout = integer(value, server.RequestTimeout);
}
}  // namespace seb::configuration::configuration_data::data_mapping
