#include "parser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace seb::server {

namespace {

QJsonObject object(const QByteArray &body, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (!document.isObject()) {
        if (error) {
            *error = parseError.errorString();
        }
        return {};
    }
    return document.object();
}

data::Instruction instructionFor(const QString &value)
{
    if (value == QStringLiteral("SEB_FORCE_LOCK_SCREEN")) return data::Instruction::LockScreen;
    if (value == QStringLiteral("NOTIFICATION_CONFIRM")) return data::Instruction::NotificationConfirm;
    if (value == QStringLiteral("SEB_PROCTORING")) return data::Instruction::Proctoring;
    if (value == QStringLiteral("SEB_RECONFIGURE_SETTINGS")) return data::Instruction::ProctoringReconfiguration;
    if (value == QStringLiteral("SEB_QUIT")) return data::Instruction::Quit;
    return data::Instruction::None;
}

}  // namespace

bool Parser::parseApi(const QByteArray &body, data::Api &api, QString *error)
{
    api = {};
    const QJsonObject root = object(body, error);
    const QJsonArray versions = root.value(QStringLiteral("api-versions")).toArray();
    for (const QJsonValue &versionValue : versions) {
        const QJsonObject version = versionValue.toObject();
        if (version.value(QStringLiteral("name")).toString() != QStringLiteral("v1")) {
            continue;
        }
        for (const QJsonValue &endpointValue : version.value(QStringLiteral("endpoints")).toArray()) {
            const QJsonObject endpoint = endpointValue.toObject();
            const QString name = endpoint.value(QStringLiteral("name")).toString();
            const QString location = endpoint.value(QStringLiteral("location")).toString();
            if (name == QStringLiteral("access-token-endpoint")) api.accessTokenEndpoint = location;
            else if (name == QStringLiteral("seb-configuration-endpoint")) api.configurationEndpoint = location;
            else if (name == QStringLiteral("seb-handshake-endpoint")) api.handshakeEndpoint = location;
            else if (name == QStringLiteral("seb-log-endpoint")) api.logEndpoint = location;
            else if (name == QStringLiteral("seb-ping-endpoint")) api.pingEndpoint = location;
        }
        if (!api.accessTokenEndpoint.isEmpty() && !api.configurationEndpoint.isEmpty() &&
            !api.handshakeEndpoint.isEmpty() && !api.pingEndpoint.isEmpty()) {
            return true;
        }
    }
    if (error && error->isEmpty()) {
        *error = QStringLiteral("The server does not advertise a complete SEB Server API v1.");
    }
    return false;
}

bool Parser::parseOAuthToken(const QByteArray &body, QString &token, QString *error)
{
    token.clear();
    const QJsonObject root = object(body, error);
    token = root.value(QStringLiteral("access_token")).toString();
    if (token.isEmpty() && error && error->isEmpty()) {
        *error = QStringLiteral("The server did not return an OAuth access token.");
    }
    return !token.isEmpty();
}

bool Parser::parseExams(const QByteArray &body, QList<data::Exam> &exams, QString *error)
{
    exams.clear();
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (!document.isArray()) {
        if (error) *error = parseError.errorString();
        return false;
    }
    for (const QJsonValue &value : document.array()) {
        const QJsonObject exam = value.toObject();
        data::Exam parsed;
        parsed.id = exam.value(QStringLiteral("examId")).toString();
        parsed.lmsName = exam.value(QStringLiteral("lmsType")).toString();
        parsed.name = exam.value(QStringLiteral("name")).toString();
        parsed.url = exam.value(QStringLiteral("url")).toString();
        if (!parsed.id.isEmpty() && !parsed.name.isEmpty() && !parsed.url.isEmpty()) {
            exams.append(parsed);
        }
    }
    if (exams.isEmpty() && error) {
        *error = QStringLiteral("The server returned no usable exams.");
    }
    return !exams.isEmpty();
}

bool Parser::parseInstruction(
    const QByteArray &body,
    data::InstructionData &instruction,
    QString *error)
{
    instruction = {};
    const QJsonObject root = object(body, error);
    const QString instructionName = root.value(QStringLiteral("instruction")).toString();
    if (instructionName.isEmpty()) {
        return false;
    }
    instruction.instruction = instructionFor(instructionName);

    const QJsonObject attributes = root.value(QStringLiteral("attributes")).toObject();
    instruction.confirmation = attributes.value(QStringLiteral("instruction-confirm")).toString();
    instruction.message = attributes.value(QStringLiteral("message")).toString();
    instruction.notificationId = attributes.value(QStringLiteral("id")).toInt();
    instruction.allowChat =
        attributes.value(QStringLiteral("jitsiMeetFeatureFlagChat")).toBool() ||
        attributes.value(QStringLiteral("zoomFeatureFlagChat")).toBool();
    instruction.receiveAudio =
        attributes.value(QStringLiteral("jitsiMeetReceiveAudio")).toBool() ||
        attributes.value(QStringLiteral("zoomReceiveAudio")).toBool();
    instruction.receiveVideo =
        attributes.value(QStringLiteral("jitsiMeetReceiveVideo")).toBool() ||
        attributes.value(QStringLiteral("zoomReceiveVideo")).toBool();
    const QString type = attributes.value(QStringLiteral("type")).toString();
    if (type.compare(QStringLiteral("lockscreen"), Qt::CaseInsensitive) == 0) {
        instruction.notificationType = data::NotificationType::LockScreen;
    } else if (type.compare(QStringLiteral("raisehand"), Qt::CaseInsensitive) == 0 ||
               type.compare(QStringLiteral("hand"), Qt::CaseInsensitive) == 0) {
        instruction.notificationType = data::NotificationType::Hand;
    }
    return true;
}

bool Parser::isTokenExpired(const QByteArray &body)
{
    QString ignored;
    const QJsonObject root = object(body, &ignored);
    return root.value(QStringLiteral("error")).toString().compare(
               QStringLiteral("invalid_token"), Qt::CaseInsensitive) == 0;
}

}  // namespace seb::server
