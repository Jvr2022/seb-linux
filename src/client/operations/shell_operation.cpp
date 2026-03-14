#include "shell_operation.h"

namespace seb::client::operations {

ShellOperation::ShellOperation(ClientContext &context, QObject *parent)
    : ClientOperation(context, parent)
{
}

OperationResult ShellOperation::perform()
{
    emit statusChanged(QStringLiteral("Initializing shell"));
    return OperationResult::Success;
}

OperationResult ShellOperation::revert()
{
    emit statusChanged(QStringLiteral("Closing shell"));
    return OperationResult::Success;
}

}  // namespace seb::client::operations
