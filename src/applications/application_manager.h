#pragma once

#include "../settings/Applications/ApplicationSettings.h"

#include <QObject>

namespace seb::applications {

class ApplicationWindow;
class ExternalApplication;

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(
        const seb::settings::applications::ApplicationSettings &settings,
        QObject *parent = nullptr);
    ~ApplicationManager() override;

    QList<ExternalApplication *> applications() const;
    void initialize();
    void autoStart();
    void terminate();

signals:
    void applicationsChanged();

private:
    seb::settings::applications::ApplicationSettings settings_;
    QList<ExternalApplication *> applications_;
};

}  // namespace seb::applications
