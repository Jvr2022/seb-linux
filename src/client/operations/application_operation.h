#pragma once

#include "client_operation.h"

namespace seb::client::operations {

class ApplicationOperation : public ClientOperation
{
    Q_OBJECT

public:
    explicit ApplicationOperation(ClientContext &context, QObject *parent = nullptr);

    OperationResult perform() override;
    OperationResult revert() override;
};

}  // namespace seb::client::operations
