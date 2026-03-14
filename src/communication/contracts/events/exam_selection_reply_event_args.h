#pragma once

#include "communication_event_args.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::events {

struct ExamSelectionReplyEventArgs : CommunicationEventArgs
{
    QUuid requestId;
    QString selectedExamId;
    bool success = false;
};

}  // namespace seb::communication::contracts::events
