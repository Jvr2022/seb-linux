#pragma once

#include "message.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::data {

struct ExamSelectionReplyMessage : Message
{
    ExamSelectionReplyMessage() = default;
    ExamSelectionReplyMessage(QUuid id, bool ok, QString selected)
        : requestId(id), selectedExamId(std::move(selected)), success(ok) {}

    QUuid requestId;
    QString selectedExamId;
    bool success = false;
};

}  // namespace seb::communication::contracts::data
