#pragma once

#include "../seb_settings.h"

#include <QObject>

namespace seb::applications {

class ApplicationWindow;
class ExternalApplication;

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(const seb::ApplicationSettings &settings, QObject *parent = nullptr);
    ~ApplicationManager() override;

    QList<ExternalApplication *> applications() const;
    void initialize();
    void autoStart();
    void terminate();

signals:
    void applicationsChanged();

private:
    seb::ApplicationSettings settings_;
    QList<ExternalApplication *> applications_;
};

}  // namespace seb::applications
