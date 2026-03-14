#pragma once

#include "client_operation.h"

#include <QList>

namespace seb::client::operations {

class ClientOperationSequence : public QObject
{
    Q_OBJECT

public:
    explicit ClientOperationSequence(QObject *parent = nullptr);

    void add(ClientOperation *operation);
    OperationResult perform();
    OperationResult revert();

signals:
    void statusChanged(const QString &status);

private:
    QList<ClientOperation *> operations_;
    QList<ClientOperation *> completed_;
};

}  // namespace seb::client::operations
