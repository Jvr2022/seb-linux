#pragma once

#include <QLockFile>
#include <QStringList>

namespace seb::security {

class SessionIntegrityStore
{
public:
    explicit SessionIntegrityStore(QString filePath);
    ~SessionIntegrityStore();

    bool prepare(const QString &configurationKey, bool hasQuitPassword, bool verify, QString *error = nullptr);
    bool activate(const QString &configurationKey, bool hasQuitPassword, bool verify, QString *error = nullptr);
    bool finish(bool cleanShutdown, QString *error = nullptr);

private:
    bool acquire(QString *error);
    bool read(QStringList &records, QString *error) const;
    bool write(const QStringList &records, QString *error) const;
    void release();

    QString filePath_;
    QLockFile lock_;
    QString activeKey_;
    QString preparedKey_;
    bool lockHeld_ = false;
    bool prepared_ = false;
    bool preparedHasQuitPassword_ = false;
    bool preparedVerify_ = false;
};

}
