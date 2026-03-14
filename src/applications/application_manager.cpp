#include "application_manager.h"

#include "application_factory.h"
#include "external_application.h"

#include <QtAlgorithms>

namespace seb::applications {

ApplicationManager::ApplicationManager(const seb::ApplicationSettings &settings, QObject *parent)
    : QObject(parent)
    , settings_(settings)
{
}

ApplicationManager::~ApplicationManager() = default;

QList<ExternalApplication *> ApplicationManager::applications() const
{
    QList<ExternalApplication *> result;
    return applications_;
}

void ApplicationManager::initialize()
{
    qDeleteAll(applications_);
    applications_.clear();

    for (const auto &settings : settings_.whitelist) {
        auto application = ApplicationFactory::tryCreate(settings);
        if (!application) {
            continue;
        }
        connect(application.get(), &ExternalApplication::windowsChanged, this, &ApplicationManager::applicationsChanged);
        applications_.push_back(application.release());
    }

    emit applicationsChanged();
}

void ApplicationManager::autoStart()
{
    for (ExternalApplication *application : std::as_const(applications_)) {
        if (application->autoStart()) {
            application->start();
        }
    }
}

void ApplicationManager::terminate()
{
    for (ExternalApplication *application : std::as_const(applications_)) {
        application->terminate();
    }
}

}  // namespace seb::applications
