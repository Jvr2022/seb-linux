#include "server_proxy.h"

#include "parser.h"
#include "sanitizer.h"

#include <QDateTime>
#include <QEventLoop>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>
#include <QUrlQuery>

namespace seb::server {

namespace {

constexpr auto AcceptHeader = "application/json, */*";
constexpr auto AuthorizationHeader = "Authorization";
constexpr auto ConnectionTokenHeader = "SEBConnectionToken";
constexpr auto ExamSaltHeader = "SEBExamSalt";
constexpr auto BrowserExamKeyHeader = "SEBServerBEK";

QString responseMessage(const QNetworkReply *reply)
{
    if (!reply) {
        return QStringLiteral("No response from SEB Server.");
    }
    if (reply->error() != QNetworkReply::NoError) {
        return reply->errorString();
    }
    return QStringLiteral("HTTP %1").arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
}

bool isSuccess(int status)
{
    return status >= 200 && status < 300;
}

}  // namespace

ServerProxy::ServerProxy(
    const seb::configuration::contracts::AppConfig &appConfig,
    QObject *parent)
    : QObject(parent)
    , appConfig_(appConfig)
{
    connect(&pingTimer_, &QTimer::timeout, this, &ServerProxy::performPing);
}

ServerProxy::~ServerProxy()
{
    stopConnectivity();
    if (connected_) {
        disconnectFromServer();
    }
}

void ServerProxy::initialize(const seb::settings::server::ServerSettings &settings)
{
    settings_ = settings;
    baseUrl_ = Sanitizer::baseAddress(settings.ServerUrl);
    api_ = {};
    connectionToken_.clear();
    oauth2Token_.clear();
    examId_.clear();
    pingNumber_ = 0;
    notificationId_ = 0;
    currentHandId_ = 0;
    currentLockScreenId_ = 0;
    connected_ = false;
}

bool ServerProxy::initializeConnection(
    const QString &api,
    const QString &connectionToken,
    const QString &examId,
    const QString &oauth2Token,
    const seb::settings::server::ServerSettings &settings,
    QString *error)
{
    initialize(settings);
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(api.toUtf8(), &parseError);
    if (!document.isObject()) {
        if (error) *error = parseError.errorString();
        return false;
    }
    const QJsonObject object = document.object();
    api_.accessTokenEndpoint = object.value(QStringLiteral("accessTokenEndpoint")).toString();
    api_.configurationEndpoint = object.value(QStringLiteral("configurationEndpoint")).toString();
    api_.handshakeEndpoint = object.value(QStringLiteral("handshakeEndpoint")).toString();
    api_.logEndpoint = object.value(QStringLiteral("logEndpoint")).toString();
    api_.pingEndpoint = object.value(QStringLiteral("pingEndpoint")).toString();
    if (api_.accessTokenEndpoint.isEmpty() || api_.configurationEndpoint.isEmpty() ||
        api_.handshakeEndpoint.isEmpty() || api_.pingEndpoint.isEmpty()) {
        if (error) *error = QStringLiteral("Stored SEB Server API information is incomplete.");
        return false;
    }
    connectionToken_ = connectionToken;
    oauth2Token_ = oauth2Token;
    examId_ = examId;
    connected_ = !connectionToken_.isEmpty() && !oauth2Token_.isEmpty();
    return connected_;
}

contracts::ServerResponse ServerProxy::connectToServer()
{
    contracts::ServerResponse result;
    if (!baseUrl_.isValid()) {
        result.message = QStringLiteral("The SEB Server URL is invalid.");
        return result;
    }

    const auto apiResponse = execute(
        QByteArrayLiteral("GET"),
        settings_.ApiEndpoint);
    if (!apiResponse.success) {
        return apiResponse;
    }

    QString error;
    if (!Parser::parseApi(apiResponse.body, api_, &error)) {
        result.message = error;
        return result;
    }
    api_.accessTokenEndpoint = Sanitizer::apiEndpoint(api_.accessTokenEndpoint);
    api_.configurationEndpoint = Sanitizer::apiEndpoint(api_.configurationEndpoint);
    api_.handshakeEndpoint = Sanitizer::apiEndpoint(api_.handshakeEndpoint);
    api_.logEndpoint = Sanitizer::apiEndpoint(api_.logEndpoint);
    api_.pingEndpoint = Sanitizer::apiEndpoint(api_.pingEndpoint);

    if (!requestOAuthToken(&error)) {
        result.message = error;
        return result;
    }
    result.success = true;
    result.statusCode = 200;
    connected_ = true;
    return result;
}

contracts::ServerResponse ServerProxy::disconnectFromServer()
{
    stopConnectivity();
    if (!connected_ || connectionToken_.isEmpty()) {
        connected_ = false;
        return {true, 200, {}, {}, {}};
    }
    const auto result = execute(
        QByteArrayLiteral("DELETE"),
        api_.handshakeEndpoint,
        QByteArrayLiteral("delete=true"),
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        true,
        true,
        false);
    connected_ = false;
    return result;
}

contracts::ServerResponse ServerProxy::availableExams(
    QList<data::Exam> &exams,
    const QString &examId)
{
    exams.clear();
    const QString user = qEnvironmentVariable("USER", QStringLiteral("unknown"));
    const QString machine = QHostInfo::localHostName();
    const QString os = QStringLiteral("Linux");
    QList<QPair<QString, QString>> values{
        {QStringLiteral("institutionId"), settings_.Institution},
        {QStringLiteral("client_id"), user},
        {QStringLiteral("seb_machine_name"), machine},
        {QStringLiteral("seb_os_name"), os},
        {QStringLiteral("seb_version"), appConfig_.programInformationalVersion}
    };
    if (!examId.trimmed().isEmpty()) {
        values.append({QStringLiteral("examId"), examId});
    }
    const auto response = execute(
        QByteArrayLiteral("POST"),
        api_.handshakeEndpoint,
        formEncode(values),
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        true);
    if (!response.success) {
        return response;
    }

    QString error;
    if (!Parser::parseExams(response.body, exams, &error)) {
        return {false, response.statusCode, error, response.body, response.headers};
    }
    const QByteArray token = firstHeader(response.headers, QByteArray(ConnectionTokenHeader));
    if (token.isEmpty()) {
        return {false, response.statusCode, QStringLiteral("The SEB Server did not return a connection token."), response.body, response.headers};
    }
    connectionToken_ = QString::fromUtf8(token);
    connected_ = true;
    return response;
}

contracts::ServerResponse ServerProxy::downloadExamConfiguration(
    const data::Exam &exam,
    const QString &path)
{
    const QString endpoint = QStringLiteral("%1?examId=%2")
        .arg(api_.configurationEndpoint, QString::fromUtf8(QUrl::toPercentEncoding(exam.id)));
    const auto response = execute(
        QByteArrayLiteral("GET"),
        endpoint,
        {},
        {},
        true,
        true);
    if (!response.success) {
        return response;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(response.body) != response.body.size() || !file.commit()) {
        return {false, response.statusCode, QStringLiteral("Could not save the exam configuration."), {}, response.headers};
    }
    return response;
}

contracts::ServerResponse ServerProxy::selectExam(
    const data::Exam &exam,
    QString *browserExamKey)
{
    if (browserExamKey) browserExamKey->clear();
    const auto response = execute(
        QByteArrayLiteral("PATCH"),
        api_.handshakeEndpoint,
        formEncode({{QStringLiteral("examId"), exam.id}}),
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        true,
        true);
    if (!response.success) {
        return response;
    }
    examId_ = exam.id;
    if (browserExamKey) {
        *browserExamKey = QString::fromUtf8(firstHeader(response.headers, QByteArray(BrowserExamKeyHeader)));
    }
    const auto salt = firstHeader(response.headers, QByteArray(ExamSaltHeader));
    Q_UNUSED(salt);
    // Linux has no protected native integrity module yet. Finish without a
    // fabricated SHA-256(token + salt) value; a server requiring ASK will
    // reject the handshake explicitly.
    return finishHandshake();
}

contracts::ServerResponse ServerProxy::sendUserIdentifier(const QString &identifier)
{
    const auto response = execute(
        QByteArrayLiteral("PATCH"),
        api_.handshakeEndpoint,
        formEncode({
            {QStringLiteral("examId"), examId_},
            {QStringLiteral("seb_user_session_id"), identifier}
        }),
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        true,
        true);
    return response;
}

contracts::ServerResponse ServerProxy::raiseHand(const QString &message)
{
    currentHandId_ = ++notificationId_;
    QJsonObject object;
    object.insert(QStringLiteral("numericValue"), currentHandId_);
    object.insert(QStringLiteral("text"), QStringLiteral("<raisehand> ") + message);
    object.insert(QStringLiteral("timestamp"), QDateTime::currentSecsSinceEpoch());
    object.insert(QStringLiteral("type"), QStringLiteral("NOTIFICATION"));
    return execute(
        QByteArrayLiteral("POST"),
        api_.logEndpoint,
        QJsonDocument(object).toJson(QJsonDocument::Compact),
        QByteArrayLiteral("application/json;charset=UTF-8"),
        true,
        true);
}

contracts::ServerResponse ServerProxy::lowerHand()
{
    QJsonObject object;
    object.insert(QStringLiteral("numericValue"), currentHandId_);
    object.insert(QStringLiteral("timestamp"), QDateTime::currentSecsSinceEpoch());
    object.insert(QStringLiteral("type"), QStringLiteral("NOTIFICATION_CONFIRMED"));
    return execute(
        QByteArrayLiteral("POST"),
        api_.logEndpoint,
        QJsonDocument(object).toJson(QJsonDocument::Compact),
        QByteArrayLiteral("application/json;charset=UTF-8"),
        true,
        true);
}

contracts::ServerResponse ServerProxy::lockScreen(const QString &message)
{
    currentLockScreenId_ = ++notificationId_;
    QJsonObject object;
    object.insert(QStringLiteral("numericValue"), currentLockScreenId_);
    object.insert(QStringLiteral("text"), QStringLiteral("<lockscreen> ") + message);
    object.insert(QStringLiteral("timestamp"), QDateTime::currentSecsSinceEpoch());
    object.insert(QStringLiteral("type"), QStringLiteral("NOTIFICATION"));
    return execute(
        QByteArrayLiteral("POST"),
        api_.logEndpoint,
        QJsonDocument(object).toJson(QJsonDocument::Compact),
        QByteArrayLiteral("application/json;charset=UTF-8"),
        true,
        true);
}

contracts::ServerResponse ServerProxy::confirmLockScreen()
{
    QJsonObject object;
    object.insert(QStringLiteral("numericValue"), currentLockScreenId_);
    object.insert(QStringLiteral("timestamp"), QDateTime::currentSecsSinceEpoch());
    object.insert(QStringLiteral("type"), QStringLiteral("NOTIFICATION_CONFIRMED"));
    return execute(
        QByteArrayLiteral("POST"),
        api_.logEndpoint,
        QJsonDocument(object).toJson(QJsonDocument::Compact),
        QByteArrayLiteral("application/json;charset=UTF-8"),
        true,
        true);
}

void ServerProxy::startConnectivity()
{
    if (!connected_ || api_.pingEndpoint.isEmpty()) {
        return;
    }
    const int interval = qMax(100, settings_.PingInterval);
    pingTimer_.setInterval(interval);
    pingTimer_.start();
}

void ServerProxy::stopConnectivity()
{
    pingTimer_.stop();
}

data::ConnectionInfo ServerProxy::connectionInfo() const
{
    QJsonObject object;
    object.insert(QStringLiteral("accessTokenEndpoint"), api_.accessTokenEndpoint);
    object.insert(QStringLiteral("configurationEndpoint"), api_.configurationEndpoint);
    object.insert(QStringLiteral("handshakeEndpoint"), api_.handshakeEndpoint);
    object.insert(QStringLiteral("logEndpoint"), api_.logEndpoint);
    object.insert(QStringLiteral("pingEndpoint"), api_.pingEndpoint);
    return {
        QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact)),
        connectionToken_,
        oauth2Token_};
}

contracts::ServerResponse ServerProxy::execute(
    const QByteArray &method,
    const QString &endpoint,
    const QByteArray &body,
    const QByteArray &contentType,
    bool authorization,
    bool connectionToken,
    bool refreshToken,
    const QHash<QByteArray, QByteArray> &additionalHeaders)
{
    contracts::ServerResponse last;
    const int attempts = qMax(1, settings_.RequestAttempts);
    bool refreshed = false;

    for (int attempt = 0; attempt < attempts; ++attempt) {
        const QUrl url = endpointUrl(endpoint);
        if (!url.isValid()) {
            last.message = QStringLiteral("The SEB Server endpoint is invalid.");
            return last;
        }

        QNetworkRequest request(url);
        request.setRawHeader(QByteArrayLiteral("Accept"), QByteArray(AcceptHeader));
        if (authorization && !oauth2Token_.isEmpty()) {
            request.setRawHeader(
                QByteArray(AuthorizationHeader),
                QByteArrayLiteral("Bearer ") + oauth2Token_.toUtf8());
        }
        if (connectionToken && !connectionToken_.isEmpty()) {
            request.setRawHeader(
                QByteArray(ConnectionTokenHeader),
                connectionToken_.toUtf8());
        }
        for (auto it = additionalHeaders.cbegin(); it != additionalHeaders.cend(); ++it) {
            request.setRawHeader(it.key(), it.value());
        }
        if (!contentType.isEmpty()) {
            request.setHeader(QNetworkRequest::ContentTypeHeader, QString::fromLatin1(contentType));
        }

        QNetworkReply *reply = nullptr;
        if (method == QByteArrayLiteral("GET")) reply = network_.get(request);
        else if (method == QByteArrayLiteral("POST")) reply = network_.post(request, body);
        else if (method == QByteArrayLiteral("PUT")) reply = network_.put(request, body);
        else reply = network_.sendCustomRequest(request, method, body);

        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        connect(&timeout, &QTimer::timeout, reply, &QNetworkReply::abort);
        timeout.start(qMax(1, settings_.RequestTimeout));
        loop.exec();

        last.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        last.body = reply->readAll();
        last.message = responseMessage(reply);
        const QList<QByteArray> names = reply->rawHeaderList();
        for (const QByteArray &name : names) {
            last.headers.insert(name, reply->rawHeader(name));
        }
        const bool ok = reply->error() == QNetworkReply::NoError && isSuccess(last.statusCode);
        reply->deleteLater();
        if (ok) {
            last.success = true;
            return last;
        }

        if (refreshToken && !refreshed && last.statusCode == 401 && Parser::isTokenExpired(last.body)) {
            refreshed = true;
            if (requestOAuthToken()) {
                --attempt;
                continue;
            }
        }

        if (attempt + 1 < attempts && settings_.RequestAttemptInterval > 0) {
            QEventLoop delayLoop;
            QTimer::singleShot(settings_.RequestAttemptInterval, &delayLoop, &QEventLoop::quit);
            delayLoop.exec();
        }
    }
    return last;
}

bool ServerProxy::requestOAuthToken(QString *error)
{
    const QByteArray credentials =
        (settings_.ClientName + QLatin1Char(':') + settings_.ClientSecret).toUtf8().toBase64();
    const auto response = execute(
        QByteArrayLiteral("POST"),
        api_.accessTokenEndpoint,
        QByteArrayLiteral("grant_type=client_credentials&scope=read%20write"),
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        false,
        false,
        false,
        {{QByteArray(AuthorizationHeader), QByteArrayLiteral("Basic ") + credentials}});
    QString parsedError;
    QString token;
    const bool parsed = response.success && Parser::parseOAuthToken(response.body, token, &parsedError);
    if (!parsed && error) {
        *error = parsedError.isEmpty() ? response.message : parsedError;
    }
    if (parsed) {
        oauth2Token_ = token;
    }
    return parsed;
}

contracts::ServerResponse ServerProxy::finishHandshake()
{
    // No Linux protected integrity module is available yet. Do not send a
    // fabricated application signature key.
    return execute(
        QByteArrayLiteral("PUT"),
        api_.handshakeEndpoint,
        {},
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        true,
        true);
}

void ServerProxy::performPing()
{
    if (!connected_ || pingInFlight_ || api_.pingEndpoint.isEmpty()) {
        return;
    }
    pingInFlight_ = true;
    QString confirmation;
    if (!instructionConfirmations_.isEmpty()) {
        confirmation = instructionConfirmations_.dequeue();
    }
    QList<QPair<QString, QString>> values{
        {QStringLiteral("timestamp"), QString::number(QDateTime::currentSecsSinceEpoch())},
        {QStringLiteral("ping-number"), QString::number(++pingNumber_)}
    };
    if (!confirmation.isEmpty()) {
        values.append({QStringLiteral("instruction-confirm"), confirmation});
    }

    const auto response = execute(
        QByteArrayLiteral("POST"),
        api_.pingEndpoint,
        formEncode(values),
        QByteArrayLiteral("application/x-www-form-urlencoded"),
        true,
        true);
    if (response.success) {
        data::InstructionData instruction;
        if (Parser::parseInstruction(response.body, instruction)) {
            handleInstruction(instruction);
            if (!instruction.confirmation.isEmpty()) {
                instructionConfirmations_.enqueue(instruction.confirmation);
            }
        }
    }
    pingInFlight_ = false;
}

void ServerProxy::handleInstruction(const data::InstructionData &instruction)
{
    switch (instruction.instruction) {
    case data::Instruction::LockScreen:
        currentLockScreenId_ = instruction.notificationId;
        emit lockScreenRequested(instruction.message);
        break;
    case data::Instruction::NotificationConfirm:
        if (instruction.notificationType == data::NotificationType::LockScreen) {
            emit lockScreenConfirmed();
        } else if (instruction.notificationType == data::NotificationType::Hand) {
            emit handConfirmed();
        }
        break;
    case data::Instruction::Proctoring:
        emit proctoringInstructionReceived(instruction.message);
        break;
    case data::Instruction::ProctoringReconfiguration:
        emit proctoringConfigurationReceived(
            instruction.allowChat,
            instruction.receiveAudio,
            instruction.receiveVideo);
        break;
    case data::Instruction::Quit:
        emit terminationRequested();
        break;
    case data::Instruction::None:
        break;
    }
}

QUrl ServerProxy::endpointUrl(const QString &endpoint) const
{
    return Sanitizer::resolve(baseUrl_, endpoint);
}

QByteArray ServerProxy::formEncode(const QList<QPair<QString, QString>> &values)
{
    QUrlQuery query;
    for (const auto &value : values) {
        query.addQueryItem(value.first, value.second);
    }
    return query.toString(QUrl::FullyEncoded).toUtf8();
}

QByteArray ServerProxy::firstHeader(
    const QHash<QByteArray, QByteArray> &headers,
    const QByteArray &name)
{
    for (auto it = headers.cbegin(); it != headers.cend(); ++it) {
        if (it.key().compare(name, Qt::CaseInsensitive) == 0) {
            return it.value();
        }
    }
    return {};
}

}  // namespace seb::server
