#pragma once

#include "InvigilationSettings.h"

#include <QString>

namespace seb::settings::server {

struct ServerSettings
{
    QString ApiEndpoint;
    QString ClientName;
    QString ClientSecret;
    QString ExamId;
    QString FallbackPasswordHash;
    QString Institution;
    InvigilationSettings Invigilation;
    bool PerformFallback = false;
    int PingInterval = 1000;
    int RequestAttemptInterval = 2000;
    int RequestAttempts = 5;
    int RequestTimeout = 30000;
    QString ServerUrl;
};

}  // namespace seb::settings::server
