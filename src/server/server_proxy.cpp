#include "server_proxy.h"

#include "parser.h"
#include "sanitizer.h"

#include <QDateTime>
#include "configuration/cryptography/key_generator.h"
#include <QPointer>
#include <memory>
#include <utility>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>

namespace seb::server {

namespace {

constexpr auto AcceptHeader = "application/json, */*";
constexpr auto AuthorizationHeader = "Authorization";
constexpr auto ConnectionTokenHeader = "SEBConnectionToken";
constexpr auto ExamSaltHeader = "SEBExamSalt";
constexpr auto BrowserExamKeyHeader = "SEBServerBEK";

bool isSuccess(int status) { return status >= 200 && status < 300; }
void complete(const ServerProxy::Completion &done, contracts::ServerResponse response)
{
    if (done) done(std::move(response));
}
} // namespace

ServerProxy::ServerProxy(const seb::configuration::contracts::AppConfig &appConfig, QObject *parent)
    : QObject(parent), appConfig_(appConfig)
{
    connect(&pingTimer_, &QTimer::timeout, this, &ServerProxy::performPing);
    logTimer_.setInterval(500);
    connect(&logTimer_, &QTimer::timeout, this, &ServerProxy::flushLogs);
}

ServerProxy::~ServerProxy()
{
    stopConnectivity();
    cancelRequests();
    clearConnection();
}

void ServerProxy::cancelRequests()
{
    ++generation_;
    const auto timers = retryTimers_;
    retryTimers_.clear();
    for (auto *timer : timers) delete timer;
    const auto replies = replies_;
    replies_.clear();
    for (auto *reply : replies) {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
    tokenWaiters_.clear();
    refreshingToken_ = false;
    pingInFlight_ = logInFlight_ = false;
}

void ServerProxy::clearConnection()
{
    state_ = ConnectionState::Disconnected;
    connectionToken_.clear();
    oauth2Token_.clear();
    examId_.clear();
    instructionConfirmations_.clear();
    api_ = {};
    settings_.ClientSecret.clear();
    currentHandId_ = pendingHandId_ = currentLockScreenId_ = 0;
    ++handRevision_;
}

void ServerProxy::initialize(const seb::settings::server::ServerSettings &settings)
{
    stopConnectivity();
    cancelRequests();
    clearConnection();
    settings_ = settings;
    baseUrl_ = Sanitizer::baseAddress(settings.ServerUrl);
    pingNumber_ = notificationId_ = 0;
    batteryPercentage_ = batteryOnline_ = -1;
    wirelessSignal_ = -2;
}

void ServerProxy::connectToServer(Completion done)
{
    execute([this, done](auto response) {
        if (!response.success) { complete(done, response); return; }
        QString error;
        if (!Parser::parseApi(response.body, api_, &error)) {
            complete(done, {false, response.statusCode, error, {}, {}}); return;
        }
        for (const auto &endpoint : {api_.accessTokenEndpoint, api_.configurationEndpoint,
             api_.handshakeEndpoint, api_.logEndpoint, api_.pingEndpoint}) {
            if (!endpointUrl(endpoint).isValid()) {
                complete(done, {false, 0, QStringLiteral("Unsafe SEB Server API endpoint."), {}, {}}); return;
            }
        }
        requestOAuthToken([this, done](auto tokenResponse) {
            if (tokenResponse.success) state_ = ConnectionState::Authenticated;
            complete(done, tokenResponse);
        });
    }, "GET", settings_.ApiEndpoint);
}

void ServerProxy::disconnectFromServer(Completion done)
{
    stopConnectivity();
    cancelRequests();
    state_ = ConnectionState::Disconnecting;
    if (connectionToken_.isEmpty()) {
        clearConnection(); complete(done, {true, 200, {}, {}, {}}); return;
    }
    execute([this, done](auto response) {
        clearConnection();
        complete(done, response);
    }, "DELETE", api_.handshakeEndpoint, "delete=true",
       "application/x-www-form-urlencoded", true, true, true);
}

void ServerProxy::availableExams(
    std::function<void(contracts::ServerResponse, QList<data::Exam>)> done, const QString &examId)
{
    QList<QPair<QString, QString>> values{
        {QStringLiteral("institutionId"), settings_.Institution},
        {QStringLiteral("client_id"), qEnvironmentVariable("USER", QStringLiteral("unknown"))},
        {QStringLiteral("seb_machine_name"), QHostInfo::localHostName()},
        {QStringLiteral("seb_os_name"), QStringLiteral("Linux")},
        {QStringLiteral("seb_version"), appConfig_.programInformationalVersion}};
    if (!examId.isEmpty()) values.append({QStringLiteral("examId"), examId});
    execute([this, done](auto response) {
        QList<data::Exam> exams;
        const auto token = firstHeader(response.headers, ConnectionTokenHeader);
        if (!token.isEmpty()) connectionToken_ = QString::fromUtf8(token);
        if (response.success) {
            QString error;
            if (connectionToken_.isEmpty() || !Parser::parseExams(response.body, exams, &error)) {
                response.success = false;
                response.message = error.isEmpty() ? QStringLiteral("Missing SEB connection token.") : error;
            } else state_ = ConnectionState::AwaitingExamSelection;
        }
        done(response, exams);
    }, "POST", api_.handshakeEndpoint, formEncode(values), "application/x-www-form-urlencoded", true,
       false);
}

void ServerProxy::downloadExamConfiguration(const data::Exam &exam, const QString &path, Completion done)
{
    QUrl url(endpointUrl(api_.configurationEndpoint));
    QString query = url.query(QUrl::FullyEncoded);
    if (!query.isEmpty()) query += QLatin1Char('&');
    query += QString::fromLatin1(formEncode({{QStringLiteral("examId"), exam.id}}));
    url.setQuery(query, QUrl::StrictMode);
    execute([path, done](auto response) {
        if (response.success) {
            QSaveFile file(path);
            if (!file.open(QIODevice::WriteOnly) || file.write(response.body) != response.body.size() || !file.commit()) {
                response.success = false;
                response.message = QStringLiteral("Could not save exam configuration.");
            }
        }
        complete(done, response);
    }, "GET", url.toString(), {}, {}, true, true);
}

void ServerProxy::selectExam(const data::Exam &exam,
    std::function<void(contracts::ServerResponse, QString)> done)
{
    if (state_ != ConnectionState::AwaitingExamSelection || connectionToken_.isEmpty()) {
        done({false, 0, QStringLiteral("No exam handshake is available."), {}, {}}, {}); return;
    }
    state_ = ConnectionState::Handshaking;
    examId_.clear();
    execute([this, exam, done](auto response) {
        if (!response.success) { state_ = ConnectionState::AwaitingExamSelection; done(response, {}); return; }
        const auto bek = QString::fromUtf8(firstHeader(response.headers, BrowserExamKeyHeader));
        QByteArray body;
        bool hasSalt = false;
        for (auto it = response.headers.cbegin(); it != response.headers.cend(); ++it)
            hasSalt |= it.key().compare(ExamSaltHeader, Qt::CaseInsensitive) == 0;
        if (hasSalt) {
            std::unique_ptr<seb::configuration::contracts::cryptography::IKeyGenerator> generator =
                std::make_unique<seb::configuration::cryptography::KeyGenerator>(appConfig_);
            const auto key = generator->calculateAppSignatureKey(connectionToken_,
                QString::fromUtf8(firstHeader(response.headers, ExamSaltHeader)));
            if (!key || key->isEmpty()) {
                state_ = ConnectionState::AwaitingExamSelection;
                done({false, 0, QStringLiteral("This server requires an App Signature Key; Linux protected integrity is unsupported."), {}, {}}, {});
                return;
            }
            body = formEncode({{QStringLiteral("seb_signature_key"), *key}});
        }
        execute([this, exam, bek, done](auto finished) {
            state_ = finished.success ? ConnectionState::Connected : ConnectionState::AwaitingExamSelection;
            if (finished.success) examId_ = exam.id;
            done(finished, finished.success ? bek : QString());
        }, "PUT", api_.handshakeEndpoint, body, "application/x-www-form-urlencoded", true, true);
    }, "PATCH", api_.handshakeEndpoint, formEncode({{QStringLiteral("examId"), exam.id}}),
       "application/x-www-form-urlencoded", true, true);
}

void ServerProxy::sendUserIdentifier(const QString &identifier, Completion done)
{
    if (!isConnected()) { complete(done, {}); return; }
    execute(done, "PATCH", api_.handshakeEndpoint,
        formEncode({{QStringLiteral("examId"), examId_}, {QStringLiteral("seb_user_session_id"), identifier}}),
        "application/x-www-form-urlencoded", true, true);
}

void ServerProxy::sendNotification(const QJsonObject &object, Completion done)
{
    if (!isConnected()) { complete(done, {}); return; }
    execute(done, "POST", api_.logEndpoint, QJsonDocument(object).toJson(QJsonDocument::Compact),
        "application/json;charset=UTF-8", true, true);
}

void ServerProxy::raiseHand(const QString &message, Completion done)
{
    const int id = ++notificationId_;
    pendingHandId_ = id;
    const auto revision = handRevision_;
    sendNotification({{QStringLiteral("numericValue"), id}, {QStringLiteral("text"), QStringLiteral("<raisehand> ") + message},
        {QStringLiteral("timestamp"), QDateTime::currentMSecsSinceEpoch()}, {QStringLiteral("type"), QStringLiteral("NOTIFICATION")}},
        [this, id, revision, done](auto response) {
            if (handRevision_ != revision) response.success = false;
            if (response.success) currentHandId_ = id;
            if (pendingHandId_ == id) pendingHandId_ = 0;
            complete(done, response);
        });
}

void ServerProxy::lowerHand(Completion done)
{
    sendNotification({{QStringLiteral("numericValue"), currentHandId_},
        {QStringLiteral("timestamp"), QDateTime::currentMSecsSinceEpoch()}, {QStringLiteral("type"), QStringLiteral("NOTIFICATION_CONFIRMED")}},
        [this, done](auto response) { if (response.success) currentHandId_ = 0; complete(done, response); });
}

void ServerProxy::lockScreen(const QString &message)
{
    currentLockScreenId_ = ++notificationId_;
    sendNotification({{QStringLiteral("numericValue"), currentLockScreenId_},
        {QStringLiteral("text"), QStringLiteral("<lockscreen> ") + message},
        {QStringLiteral("timestamp"), QDateTime::currentMSecsSinceEpoch()}, {QStringLiteral("type"), QStringLiteral("NOTIFICATION")}});
}

void ServerProxy::confirmLockScreen()
{
    sendNotification({{QStringLiteral("numericValue"), currentLockScreenId_},
        {QStringLiteral("timestamp"), QDateTime::currentMSecsSinceEpoch()}, {QStringLiteral("type"), QStringLiteral("NOTIFICATION_CONFIRMED")}});
}

void ServerProxy::startConnectivity()
{
    if (!isConnected() || pingTimer_.isActive()) return;
    pingTimer_.start(qMax(100, settings_.PingInterval));
    logTimer_.start();
    logEvent(QStringLiteral("Browser session started"));
}

void ServerProxy::stopConnectivity()
{
    pingTimer_.stop(); logTimer_.stop(); logs_.clear();
}

void ServerProxy::logEvent(const QString &text, const QString &severity)
{
    if (!isConnected() || !logTimer_.isActive()) return;
    QString safe = text.left(2048);
    for (const auto &secret : {oauth2Token_, connectionToken_, settings_.ClientSecret})
        if (!secret.isEmpty()) safe.replace(secret, QStringLiteral("[redacted]"));
    if (logs_.size() >= 1000) logs_.dequeue();
    logs_.enqueue({{QStringLiteral("text"), safe}, {QStringLiteral("timestamp"), QDateTime::currentMSecsSinceEpoch()},
        {QStringLiteral("type"), severity == QStringLiteral("ERROR") ? QStringLiteral("ERROR_LOG") :
            severity == QStringLiteral("WARNING") ? QStringLiteral("WARN_LOG") :
            severity == QStringLiteral("DEBUG") ? QStringLiteral("DEBUG_LOG") : QStringLiteral("INFO_LOG")}});
}

void ServerProxy::flushLogs()
{
    if (!isConnected() || logInFlight_ || logs_.isEmpty()) return;
    logInFlight_ = true;
    auto remaining = std::make_shared<int>(qMin(10, int(logs_.size())));
    auto next = std::make_shared<std::function<void()>>();
    std::weak_ptr<std::function<void()>> weak = next;
    *next = [this, remaining, weak] {
        auto keep = weak.lock();
        if (!keep || !logTimer_.isActive() || logs_.isEmpty() || (*remaining)-- <= 0) { logInFlight_ = false; return; }
        const auto entry = logs_.head();
        sendNotification(entry, [this, keep, entry](auto response) {
            if (!response.success) { logInFlight_ = false; return; }
            if (!logs_.isEmpty() && logs_.head() == entry) logs_.dequeue();
            (*keep)();
        });
    };
    (*next)();
}

void ServerProxy::reportBattery(int percentage, bool online)
{
    if (!isConnected() || (batteryPercentage_ == percentage && batteryOnline_ == int(online))) return;
    if (!logTimer_.isActive()) return;
    batteryPercentage_ = percentage; batteryOnline_ = int(online);
    logEvent(QStringLiteral("<battery> %1% (%2)").arg(percentage).arg(online ? QStringLiteral("connected") : QStringLiteral("disconnected")));
    if (!logs_.isEmpty()) logs_.last().insert(QStringLiteral("numericValue"), percentage);
}

void ServerProxy::reportWireless(int signal)
{
    if (!isConnected() || (wirelessSignal_ == signal) ||
        (signal >= 0 && wirelessSignal_ >= 0 && qAbs(signal - wirelessSignal_) < 5)) return;
    if (!logTimer_.isActive()) return;
    wirelessSignal_ = signal;
    logEvent(signal < 0 ? QStringLiteral("<wlan> disconnected") : QStringLiteral("<wlan> %1%").arg(signal));
    if (signal >= 0 && !logs_.isEmpty()) logs_.last().insert(QStringLiteral("numericValue"), signal);
}

void ServerProxy::execute(Completion done, const QByteArray &method, const QString &endpoint,
    const QByteArray &body, const QByteArray &contentType, bool authorization, bool connectionToken,
    bool refreshToken, const QHash<QByteArray, QByteArray> &additionalHeaders)
{
    struct Operation { int attempts = 0; bool refreshed = false; };
    const auto operation = std::make_shared<Operation>();
    const auto generation = generation_;
    auto send = std::make_shared<std::function<void()>>();
    std::weak_ptr<std::function<void()>> weak = send;
    *send = [=, this] {
        auto keep = weak.lock();
        if (!keep || generation != generation_) return;
        const QUrl url = endpointUrl(endpoint);
        if (!url.isValid()) { complete(done, {false, 0, QStringLiteral("Unsafe SEB Server endpoint."), {}, {}}); return; }
        const QString usedToken = oauth2Token_;
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
        request.setRawHeader("Accept", AcceptHeader);
        request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
        request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
        if (authorization) request.setRawHeader(AuthorizationHeader, "Bearer " + usedToken.toUtf8());
        if (connectionToken) request.setRawHeader(ConnectionTokenHeader, connectionToken_.toUtf8());
        if (!contentType.isEmpty()) request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
        for (auto it = additionalHeaders.cbegin(); it != additionalHeaders.cend(); ++it) request.setRawHeader(it.key(), it.value());
        ++operation->attempts;
        auto *reply = network_.sendCustomRequest(request, method, body);
        replies_.insert(reply);
        auto *timeout = new QTimer(reply);
        timeout->setSingleShot(true);
        connect(timeout, &QTimer::timeout, reply, [reply] { reply->setProperty("sebTimedOut", true); reply->abort(); });
        timeout->start(qMax(1, settings_.RequestTimeout));
        connect(reply, &QNetworkReply::finished, this, [=, this] {
            timeout->stop(); replies_.remove(reply);
            contracts::ServerResponse response;
            response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            response.body = reply->readAll();
            for (const auto &pair : reply->rawHeaderPairs()) response.headers.insert(pair.first, pair.second);
            response.success = reply->error() == QNetworkReply::NoError && isSuccess(response.statusCode);
            response.message = reply->property("sebTimedOut").toBool() ? QStringLiteral("SEB Server request timed out.") :
                response.statusCode == 401 ? QStringLiteral("SEB Server authentication failed.") :
                response.statusCode ? QStringLiteral("SEB Server HTTP %1.").arg(response.statusCode) : QStringLiteral("SEB Server transport error.");
            reply->deleteLater();
            if (generation != generation_) return;
            if (response.success) { complete(done, response); return; }
            if (authorization && refreshToken && !operation->refreshed && response.statusCode == 401 && Parser::isTokenExpired(response.body)) {
                operation->refreshed = true;
                --operation->attempts;
                if (usedToken != oauth2Token_) { (*keep)(); return; }
                requestOAuthToken([=, this](auto tokenResponse) {
                    if (generation != generation_) return;
                    if (tokenResponse.success) (*keep)(); else complete(done, tokenResponse);
                });
                return;
            }
            const bool transient = response.statusCode == 0 || response.statusCode == 408 || response.statusCode == 429 || response.statusCode >= 500;
            if (transient && operation->attempts < qMax(1, settings_.RequestAttempts)) {
                auto *retry = new QTimer(this);
                retry->setSingleShot(true);
                retryTimers_.insert(retry);
                connect(retry, &QTimer::timeout, this, [this, retry, keep] {
                    retryTimers_.remove(retry); retry->deleteLater(); (*keep)();
                });
                retry->start(qMax(0, settings_.RequestAttemptInterval));
            } else complete(done, response);
        });
    };
    (*send)();
}

void ServerProxy::requestOAuthToken(Completion done)
{
    tokenWaiters_.append(done);
    if (refreshingToken_) return;
    refreshingToken_ = true;
    const QByteArray credentials = (settings_.ClientName + QLatin1Char(':') + settings_.ClientSecret).toUtf8().toBase64();
    execute([this](auto response) {
        QString token, error;
        if (response.success && !Parser::parseOAuthToken(response.body, token, &error)) {
            response.success = false; response.message = QStringLiteral("Malformed OAuth token response.");
        }
        if (response.success) oauth2Token_ = token;
        refreshingToken_ = false;
        const auto waiters = std::exchange(tokenWaiters_, {});
        const auto generation = generation_;
        QPointer<ServerProxy> self(this);
        for (const auto &waiter : waiters) {
            if (!self || generation != self->generation_) break;
            complete(waiter, response);
        }
    }, "POST", api_.accessTokenEndpoint, "grant_type=client_credentials&scope=read%20write",
       "application/x-www-form-urlencoded", false, false, false,
       {{QByteArray(AuthorizationHeader), "Basic " + credentials}});
}

void ServerProxy::performPing()
{
    if (!isConnected() || pingInFlight_) return;
    pingInFlight_ = true;
    const QString confirmation = instructionConfirmations_.isEmpty() ? QString() : instructionConfirmations_.head();
    QList<QPair<QString, QString>> values{
        {QStringLiteral("timestamp"), QString::number(QDateTime::currentMSecsSinceEpoch())},
        {QStringLiteral("ping-number"), QString::number(++pingNumber_)}};
    if (!confirmation.isEmpty()) values.append({QStringLiteral("instruction-confirm"), confirmation});
    execute([this, confirmation](auto response) {
        pingInFlight_ = false;
        if (!isConnected()) return;
        if (!response.success) {
            if (response.statusCode >= 400 && response.statusCode < 500 &&
                response.statusCode != 408 && response.statusCode != 429) {
                stopConnectivity();
                emit terminationRequested();
            }
            return;
        }
        if (!confirmation.isEmpty() && !instructionConfirmations_.isEmpty()) instructionConfirmations_.dequeue();
        data::InstructionData instruction;
        if (Parser::parseInstruction(response.body, instruction)) {
            const bool supported = instruction.instruction != data::Instruction::Proctoring &&
                instruction.instruction != data::Instruction::ProctoringReconfiguration;
            handleInstruction(instruction);
            if (supported && !instruction.confirmation.isEmpty() && !instructionConfirmations_.contains(instruction.confirmation))
                instructionConfirmations_.enqueue(instruction.confirmation);
        }
    }, "POST", api_.pingEndpoint, formEncode(values), "application/x-www-form-urlencoded", true, true);
}

void ServerProxy::handleInstruction(const data::InstructionData &instruction)
{
    switch (instruction.instruction) {
    case data::Instruction::LockScreen:
        emit lockScreenRequested(instruction.message); break;
    case data::Instruction::NotificationConfirm:
        if (instruction.notificationType == data::NotificationType::LockScreen && instruction.notificationId == currentLockScreenId_) {
            currentLockScreenId_ = 0;
            emit lockScreenConfirmed();
        }
        else if (instruction.notificationType == data::NotificationType::Hand && (instruction.notificationId == currentHandId_ || instruction.notificationId == pendingHandId_)) {
            currentHandId_ = pendingHandId_ = 0;
            ++handRevision_; emit handConfirmed();
        }
        break;
    case data::Instruction::Proctoring:
    case data::Instruction::ProctoringReconfiguration:
        logEvent(QStringLiteral("Requested proctoring capability is unavailable on Linux."), QStringLiteral("ERROR"));
        emit lockScreenRequested(QStringLiteral("The server requires an unsupported proctoring capability. Contact your invigilator."));
        break;
    case data::Instruction::Quit: emit terminationRequested(); break;
    case data::Instruction::None: break;
    }
}

QUrl ServerProxy::endpointUrl(const QString &endpoint) const
{
    return Sanitizer::resolve(baseUrl_, endpoint);
}

QByteArray ServerProxy::formEncode(const QList<QPair<QString, QString>> &values)
{
    QByteArray body;
    for (const auto &value : values) {
        if (!body.isEmpty()) body += '&';
        body += QUrl::toPercentEncoding(value.first) + '=' + QUrl::toPercentEncoding(value.second);
    }
    return body;
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
