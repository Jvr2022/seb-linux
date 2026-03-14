#pragma once

#include "../data/exam_selection_request_message.h"
#include "communication_event_args.h"

namespace seb::communication::contracts::events {

struct ExamSelectionRequestEventArgs : CommunicationEventArgs
{
    QList<seb::communication::contracts::data::Exam> exams;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::events
