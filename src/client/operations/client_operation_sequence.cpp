#include "client_operation_sequence.h"

namespace seb::client::operations {

ClientOperationSequence::ClientOperationSequence(QObject *parent)
    : QObject(parent)
{
}

void ClientOperationSequence::add(ClientOperation *operation)
{
    if (!operation) {
        return;
    }
    connect(operation, &ClientOperation::statusChanged, this, &ClientOperationSequence::statusChanged);
    operations_.push_back(operation);
}

OperationResult ClientOperationSequence::perform()
{
    completed_.clear();
    for (auto *operation : operations_) {
        const auto result = operation->perform();
        if (result != OperationResult::Success) {
            return result;
        }
        completed_.push_back(operation);
    }
    return OperationResult::Success;
}

OperationResult ClientOperationSequence::revert()
{
    for (auto it = completed_.crbegin(); it != completed_.crend(); ++it) {
        const auto result = (*it)->revert();
        if (result != OperationResult::Success) {
            return result;
        }
    }
    return OperationResult::Success;
}

}  // namespace seb::client::operations
