#pragma once

#include "contracts/application_window.h"

#include <QObject>
#include <QString>

namespace seb::applications {

class ApplicationWindow : public QObject, public contracts::IApplicationWindow
{
    Q_OBJECT

public:
    explicit ApplicationWindow(QString windowId, QString title, QString iconPath, QObject *parent = nullptr);

    QString handle() const override;
    QString iconPath() const override;
    QString title() const override;
    QString windowId() const;
    void activate() override;
    void update(const QString &title, const QString &iconPath);

signals:
    void changed();

private:
    QString windowId_;
    QString title_;
    QString iconPath_;
};

}  // namespace seb::applications
