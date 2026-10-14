#pragma once

#include "InvigilationSettings.h"

#include <QString>

namespace seb::settings::server {

struct ServerSettings
{
    bool Enabled = false;
    QString ApiEndpoint;
    QString ApiKey;
    QString ClientId;
    QString ClientSecret;
    QString ExamId;
    QString FallbackPasswordHash;
    QString Host;
    QString Institution;
    InvigilationSettings Invigilation;
    int Port = 0;
    bool PerformFallback = false;
    int PingInterval = 1000;
    int RequestAttemptInterval = 2000;
    int RequestAttempts = 5;
    int RequestTimeout = 30000;
    QString Url;
};

}  // namespace seb::settings::server
