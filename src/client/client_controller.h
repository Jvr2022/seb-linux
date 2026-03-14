#pragma once

#include "client_context.h"
#include "operations/client_operation_sequence.h"

#include <QObject>

namespace seb::client {

class ClientController : public QObject
{
    Q_OBJECT

public:
    explicit ClientController(ClientContext &context, QObject *parent = nullptr);

    operations::ClientOperationSequence &operations();
    bool tryStart();
    void terminate();

private:
    void delegateStartupResponsibilities();
    void delegateShutdownResponsibilities();

    ClientContext &context_;
    operations::ClientOperationSequence operations_;
};

}  // namespace seb::client
