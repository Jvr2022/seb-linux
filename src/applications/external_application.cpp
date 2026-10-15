#include "external_application.h"

#include "application_window.h"
#include "external_application_instance.h"

#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace seb::applications {

ExternalApplication::ExternalApplication(
    const seb::settings::applications::WhitelistApplication &settings,
    QObject *parent)
    : QObject(parent)
    , settings_(settings)
{
}

ExternalApplication::~ExternalApplication()
{
    qDeleteAll(instances_);
}

bool ExternalApplication::autoStart() const
{
    return settings_.AutoStart;
}

QString ExternalApplication::executablePath() const
{
    return resolvedExecutablePath_;
}

QString ExternalApplication::iconPath() const
{
    return resolvedExecutablePath_.isEmpty() ? settings_.ExecutableName : resolvedExecutablePath_;
}

QString ExternalApplication::id() const
{
    return settings_.ExecutableName;
}

QString ExternalApplication::name() const
{
    return settings_.DisplayName.isEmpty()
        ? QFileInfo(settings_.ExecutableName).completeBaseName()
        : settings_.DisplayName;
}

QString ExternalApplication::tooltip() const
{
    return settings_.Description.isEmpty() ? name() : settings_.Description;
}

bool ExternalApplication::showInShell() const
{
    return settings_.ShowInShell;
}

QList<ApplicationWindow *> ExternalApplication::typedWindows() const
{
    QList<ApplicationWindow *> result;
    for (ExternalApplicationInstance *instance : instances_) {
        result.append(instance->windows());
    }
    return result;
}

QList<contracts::IApplicationWindow *> ExternalApplication::windows() const
{
    QList<contracts::IApplicationWindow *> result;
    for (auto *window : typedWindows()) {
        result.push_back(window);
    }
    return result;
}

void ExternalApplication::initialize()
{
    resolvedExecutablePath_ = settings_.ExecutablePath;
    if (resolvedExecutablePath_.isEmpty()) {
        resolvedExecutablePath_ = QStandardPaths::findExecutable(settings_.ExecutableName);
    } else if (QFileInfo(resolvedExecutablePath_).isDir()) {
        resolvedExecutablePath_ = QFileInfo(
            resolvedExecutablePath_ + QLatin1Char('/') + settings_.ExecutableName).absoluteFilePath();
    }
}

void ExternalApplication::start()
{
    if (resolvedExecutablePath_.isEmpty()) {
        initialize();
    }
    if (resolvedExecutablePath_.isEmpty()) {
        return;
    }

    auto *process = new QProcess(this);
    process->setProgram(resolvedExecutablePath_);
    process->setArguments(arguments());
    process->start();
    if (!process->waitForStarted(3000)) {
        process->deleteLater();
        return;
    }

    auto *instance = new ExternalApplicationInstance(process->processId(), resolvedExecutablePath_, this);
    instance->attachProcess(process);
    instance->startMonitoring(1000);
    connect(instance, &ExternalApplicationInstance::terminated, this, &ExternalApplication::handleInstanceTerminated);
    connect(instance, &ExternalApplicationInstance::windowsChanged, this, &ExternalApplication::windowsChanged);
    instances_.push_back(instance);
    emit windowsChanged();
}

void ExternalApplication::terminate()
{
    for (ExternalApplicationInstance *instance : std::as_const(instances_)) {
        instance->terminate(!settings_.AllowRunning || settings_.AutoTerminate);
    }
}

void ExternalApplication::handleInstanceTerminated(qint64 processId)
{
    for (int index = 0; index < instances_.size(); ++index) {
        if (instances_.at(index)->processId() == processId) {
            delete instances_.takeAt(index);
            emit windowsChanged();
            return;
        }
    }
}

QStringList ExternalApplication::arguments() const
{
    QStringList args;
    for (const QString &argument : settings_.Arguments) {
        args.push_back(QProcessEnvironment::systemEnvironment().value(argument, argument));
    }
    return args;
}

}  // namespace seb::applications
