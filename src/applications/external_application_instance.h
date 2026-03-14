#pragma once

#include <QObject>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QProcess;
class QTimer;
QT_END_NAMESPACE

namespace seb::applications {

class ApplicationWindow;

class ExternalApplicationInstance : public QObject
{
    Q_OBJECT

public:
    explicit ExternalApplicationInstance(qint64 processId, QString executablePath, QObject *parent = nullptr);
    ~ExternalApplicationInstance() override;

    QList<ApplicationWindow *> windows() const;
    qint64 processId() const;
    void attachProcess(QProcess *process);
    void startMonitoring(int intervalMs);
    void terminate(bool force);

signals:
    void terminated(qint64 processId);
    void windowsChanged();

private:
    void refreshWindows();
    QString resolveWindowTitle(const QString &windowId) const;

    qint64 processId_ = 0;
    QString executablePath_;
    QString fallbackIconPath_;
    QList<ApplicationWindow *> windows_;
    QProcess *process_ = nullptr;
    QTimer *timer_ = nullptr;
};

}  // namespace seb::applications
