#include "client_operation.h"

namespace seb::client::operations {

ClientOperation::ClientOperation(ClientContext &context, QObject *parent)
    : QObject(parent)
    , context_(context)
{
}

}  // namespace seb::client::operations
