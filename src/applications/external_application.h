#pragma once

#include "contracts/application.h"
#include "../seb_settings.h"

#include <QObject>
#include <QStringList>

namespace seb::applications {

class ApplicationWindow;
class ExternalApplicationInstance;

class ExternalApplication : public QObject, public contracts::IApplication
{
    Q_OBJECT

public:
    explicit ExternalApplication(const seb::WhitelistApplicationSettings &settings, QObject *parent = nullptr);
    ~ExternalApplication() override;

    bool autoStart() const override;
    QString executablePath() const;
    QString iconPath() const override;
    QString id() const override;
    QString name() const override;
    QString tooltip() const override;
    bool showInShell() const;
    QList<ApplicationWindow *> typedWindows() const;
    QList<contracts::IApplicationWindow *> windows() const override;

    void initialize() override;
    void start() override;
    void terminate() override;

signals:
    void windowsChanged();

private:
    void handleInstanceTerminated(qint64 processId);
    QStringList arguments() const;

    seb::WhitelistApplicationSettings settings_;
    QString resolvedExecutablePath_;
    QList<ExternalApplicationInstance *> instances_;
};

}  // namespace seb::applications
