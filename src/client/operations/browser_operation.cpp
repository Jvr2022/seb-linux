#include "browser_operation.h"

namespace seb::client::operations {

BrowserOperation::BrowserOperation(ClientContext &context, QObject *parent)
    : ClientOperation(context, parent)
{
}

OperationResult BrowserOperation::perform()
{
    emit statusChanged(QStringLiteral("Initializing browser"));
    if (context_.browser) {
        context_.browser->initialize();
    }
    return OperationResult::Success;
}

OperationResult BrowserOperation::revert()
{
    emit statusChanged(QStringLiteral("Terminating browser"));
    if (context_.browser) {
        context_.browser->terminate();
    }
    return OperationResult::Success;
}

}  // namespace seb::client::operations
