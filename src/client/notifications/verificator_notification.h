#pragma once

#include <QObject>

namespace seb::client::notifications {

class VerificatorNotification : public QObject
{
    Q_OBJECT

public:
    explicit VerificatorNotification(QObject *parent = nullptr);

    QString iconPath() const;
    QString tooltip() const;

public slots:
    void activate();
    void terminate();

signals:
    void activationRequested();
    void terminationRequested();
};

}  // namespace seb::client::notifications
