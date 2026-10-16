#pragma once

#include "data/api.h"
#include "data/exam.h"
#include "data/instructions.h"

#include <QByteArray>
#include <QList>

namespace seb::server {

class Parser
{
public:
    static bool parseApi(const QByteArray &body, data::Api &api, QString *error = nullptr);
    static bool parseOAuthToken(const QByteArray &body, QString &token, QString *error = nullptr);
    static bool parseExams(const QByteArray &body, QList<data::Exam> &exams, QString *error = nullptr);
    static bool parseInstruction(
        const QByteArray &body,
        data::InstructionData &instruction,
        QString *error = nullptr);
    static bool isTokenExpired(const QByteArray &body);
};

}  // namespace seb::server
