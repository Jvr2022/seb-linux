#include "session_integrity_store.h"

#include <QCryptographicHash>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QRegularExpression>
#include <unistd.h>

namespace seb::security {
namespace {
const QByteArray fileMagic = QByteArrayLiteral("SEB-LINUX-SESSION-HISTORY");
constexpr quint16 fileVersion = 1;
constexpr quint32 maximumRecords = 4096;
constexpr qint64 maximumFileSize = 4 * 1024 * 1024;

bool fail(QString *error, const QString &message)
{
    if (error) *error = message;
    return false;
}

bool validKey(const QString &key)
{
    static const QRegularExpression pattern(QStringLiteral("^[0-9a-fA-F]{64}$"));
    return pattern.match(key).hasMatch();
}

QFileDevice::Permissions privateFilePermissions()
{
    return QFileDevice::ReadOwner | QFileDevice::WriteOwner;
}

QFileDevice::Permissions privateDirectoryPermissions()
{
    return QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner;
}
}

SessionIntegrityStore::SessionIntegrityStore(QString filePath)
    : filePath_(std::move(filePath))
    , lock_(filePath_ + QStringLiteral(".lock"))
{
    lock_.setStaleLockTime(0);
}

SessionIntegrityStore::~SessionIntegrityStore()
{
    release();
}

bool SessionIntegrityStore::prepare(
    const QString &configurationKey,
    bool hasQuitPassword,
    bool verify,
    QString *error)
{
    prepared_ = false;
    preparedKey_ = configurationKey;
    preparedHasQuitPassword_ = hasQuitPassword;
    preparedVerify_ = verify;

    if ((hasQuitPassword || lockHeld_) && !acquire(error)) return false;
    if (verify && hasQuitPassword) {
        if (!validKey(configurationKey))
            return fail(error, QStringLiteral("Session verification requires a valid configuration key."));
        QStringList records;
        if (!read(records, error)) return false;
        if (records.contains(configurationKey) && configurationKey != activeKey_)
            return fail(error, QStringLiteral("This configuration was found in unfinished session history. The previous exam session did not shut down cleanly."));
    }

    prepared_ = true;
    return true;
}

bool SessionIntegrityStore::activate(
    const QString &configurationKey,
    bool hasQuitPassword,
    bool verify,
    QString *error)
{
    if (!prepared_ || preparedKey_ != configurationKey ||
        preparedHasQuitPassword_ != hasQuitPassword || preparedVerify_ != verify)
        return fail(error, QStringLiteral("Session integrity was not prepared for this configuration."));

    if (!hasQuitPassword && !lockHeld_) {
        prepared_ = false;
        return true;
    }
    if (!acquire(error)) return false;

    QStringList records;
    if (!read(records, error)) return false;
    if (verify && hasQuitPassword && records.contains(configurationKey) && configurationKey != activeKey_)
        return fail(error, QStringLiteral("This configuration was found in unfinished session history. The previous exam session did not shut down cleanly."));

    if (!activeKey_.isEmpty()) records.removeAll(activeKey_);
    if (verify && hasQuitPassword && !records.contains(configurationKey)) records.append(configurationKey);
    if (!write(records, error)) return false;

    activeKey_ = hasQuitPassword ? configurationKey : QString();
    prepared_ = false;
    if (activeKey_.isEmpty()) release();
    return true;
}

bool SessionIntegrityStore::finish(bool cleanShutdown, QString *error)
{
    if (!lockHeld_) return true;
    if (!cleanShutdown) {
        release();
        return true;
    }

    QStringList records;
    if (!read(records, error)) {
        release();
        return false;
    }
    if (!activeKey_.isEmpty()) records.removeAll(activeKey_);
    const bool success = write(records, error);
    activeKey_.clear();
    prepared_ = false;
    release();
    return success;
}

bool SessionIntegrityStore::acquire(QString *error)
{
    if (lockHeld_) return true;
    const QFileInfo info(filePath_);
    const QString directory = info.absolutePath();
    if (!QDir().mkpath(directory))
        return fail(error, QStringLiteral("Could not create the session history directory."));
    if (QFileInfo(directory).isSymLink() || !QFile::setPermissions(directory, privateDirectoryPermissions()))
        return fail(error, QStringLiteral("Could not secure the session history directory permissions."));
    if (!lock_.tryLock(0))
        return fail(error, QStringLiteral("Another Safe Exam Browser session is using session integrity storage."));
    lockHeld_ = true;
    return true;
}

bool SessionIntegrityStore::read(QStringList &records, QString *error) const
{
    records.clear();
    QFile file(filePath_);
    if (!file.exists()) return true;
    const QFileInfo info(filePath_);
    const auto permissions = info.permissions();
    const auto sharedPermissions = QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
        QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;
    if (info.isSymLink() || info.ownerId() != static_cast<uint>(geteuid()) || (permissions & sharedPermissions))
        return fail(error, QStringLiteral("Protected session history has unsafe ownership or permissions."));
    if (!file.open(QIODevice::ReadOnly))
        return fail(error, QStringLiteral("Could not read protected session history."));
    if (file.size() <= 0 || file.size() > maximumFileSize)
        return fail(error, QStringLiteral("Protected session history has an invalid size."));

    QByteArray magic;
    quint16 version = 0;
    QByteArray payload;
    QByteArray checksum;
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_6_0);
    stream >> magic >> version >> payload >> checksum;
    if (stream.status() != QDataStream::Ok || !stream.atEnd() || magic != fileMagic || version != fileVersion ||
        QCryptographicHash::hash(payload, QCryptographicHash::Sha256) != checksum)
        return fail(error, QStringLiteral("Protected session history is corrupted or unsupported."));

    QDataStream data(payload);
    data.setVersion(QDataStream::Qt_6_0);
    quint32 count = 0;
    data >> count;
    if (data.status() != QDataStream::Ok || count > maximumRecords)
        return fail(error, QStringLiteral("Protected session history contains invalid records."));
    for (quint32 index = 0; index < count; ++index) {
        QString key;
        data >> key;
        if (data.status() != QDataStream::Ok || !validKey(key))
            return fail(error, QStringLiteral("Protected session history contains an invalid configuration key."));
        if (!records.contains(key)) records.append(key);
    }
    if (!data.atEnd()) return fail(error, QStringLiteral("Protected session history contains trailing data."));
    return true;
}

bool SessionIntegrityStore::write(const QStringList &records, QString *error) const
{
    if (records.size() > static_cast<qsizetype>(maximumRecords))
        return fail(error, QStringLiteral("Protected session history exceeds its record limit."));

    QByteArray payload;
    QDataStream data(&payload, QIODevice::WriteOnly);
    data.setVersion(QDataStream::Qt_6_0);
    data << static_cast<quint32>(records.size());
    for (const auto &key : records) {
        if (!validKey(key)) return fail(error, QStringLiteral("Refusing to write an invalid configuration key to session history."));
        data << key;
    }
    if (data.status() != QDataStream::Ok)
        return fail(error, QStringLiteral("Could not encode protected session history."));

    QSaveFile file(filePath_);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return fail(error, QStringLiteral("Could not open protected session history for writing."));
    if (!file.setPermissions(privateFilePermissions())) {
        file.cancelWriting();
        return fail(error, QStringLiteral("Could not secure protected session history permissions."));
    }
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << fileMagic << fileVersion << payload << QCryptographicHash::hash(payload, QCryptographicHash::Sha256);
    if (stream.status() != QDataStream::Ok || !file.commit())
        return fail(error, QStringLiteral("Could not safely update protected session history."));
    return true;
}

void SessionIntegrityStore::release()
{
    if (lockHeld_) lock_.unlock();
    lockHeld_ = false;
}

}
