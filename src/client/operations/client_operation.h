#pragma once

#include "../client_context.h"

#include <QObject>

namespace seb::client::operations {

enum class OperationResult
{
    Success,
    Failed,
    Aborted
};

class ClientOperation : public QObject
{
    Q_OBJECT

public:
    explicit ClientOperation(ClientContext &context, QObject *parent = nullptr);

    virtual OperationResult perform() = 0;
    virtual OperationResult revert() = 0;

signals:
    void statusChanged(const QString &status);

protected:
    ClientContext &context_;
};

}  // namespace seb::client::operations
