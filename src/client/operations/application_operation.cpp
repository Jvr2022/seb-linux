#include "application_operation.h"

namespace seb::client::operations {

ApplicationOperation::ApplicationOperation(ClientContext &context, QObject *parent)
    : ClientOperation(context, parent)
{
}

OperationResult ApplicationOperation::perform()
{
    emit statusChanged(QStringLiteral("Initializing applications"));
    for (auto *application : context_.applications) {
        if (application) {
            application->initialize();
        }
    }
    return OperationResult::Success;
}

OperationResult ApplicationOperation::revert()
{
    emit statusChanged(QStringLiteral("Finalizing applications"));
    for (auto *application : context_.applications) {
        if (application) {
            application->terminate();
        }
    }
    return OperationResult::Success;
}

}  // namespace seb::client::operations
