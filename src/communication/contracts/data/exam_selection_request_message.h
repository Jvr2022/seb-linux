#pragma once

#include "message.h"

#include <QList>
#include <QString>
#include <QUuid>

namespace seb::communication::contracts::data {

struct Exam
{
    QString id;
    QString lms;
    QString name;
    QString url;
};

struct ExamSelectionRequestMessage : Message
{
    ExamSelectionRequestMessage() = default;
    ExamSelectionRequestMessage(QList<Exam> examList, QUuid id)
        : exams(std::move(examList)), requestId(id) {}

    QList<Exam> exams;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::data
