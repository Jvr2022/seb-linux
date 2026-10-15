#include "application_factory.h"

#include "external_application.h"

#include <QFileInfo>
#include <QStandardPaths>

namespace seb::applications {

std::unique_ptr<ExternalApplication> ApplicationFactory::tryCreate(
    const seb::settings::applications::WhitelistApplication &settings)
{
    auto application = std::make_unique<ExternalApplication>(settings);
    application->initialize();
    if (application->executablePath().isEmpty() &&
        QStandardPaths::findExecutable(settings.ExecutableName).isEmpty()) {
        return {};
    }
    return application;
}

}  // namespace seb::applications
