#pragma once

#include "contracts/server_response.h"
#include "data/api.h"
#include "data/exam.h"
#include "data/instructions.h"

#include "configuration/contracts/app_config.h"
#include "settings/Server/ServerSettings.h"

#include <functional>
#include <QJsonObject>
#include <QSet>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QHash>
#include <QString>
#include <QTimer>
#include <QUrl>

namespace seb::server {

class ServerProxy : public QObject
{
    Q_OBJECT

public:
    explicit ServerProxy(
        const seb::configuration::contracts::AppConfig &appConfig,
        QObject *parent = nullptr);
    ~ServerProxy() override;

    void initialize(const seb::settings::server::ServerSettings &settings);
    using Completion = std::function<void(contracts::ServerResponse)>;
    enum class ConnectionState { Disconnected, Authenticated, AwaitingExamSelection,
                                 Handshaking, Connected, Disconnecting };
    void connectToServer(Completion done);
    void disconnectFromServer(Completion done = {});
    void availableExams(std::function<void(contracts::ServerResponse, QList<data::Exam>)> done,
                        const QString &examId = {});
    void downloadExamConfiguration(const data::Exam &exam, const QString &path, Completion done);
    void selectExam(const data::Exam &exam,
                    std::function<void(contracts::ServerResponse, QString)> done);
    void sendUserIdentifier(const QString &identifier, Completion done = {});
    void raiseHand(const QString &message, Completion done);
    void lowerHand(Completion done);
    void lockScreen(const QString &message = {});
    void confirmLockScreen();
    void logEvent(const QString &text, const QString &severity = QStringLiteral("INFO"));
    void reportBattery(int percentage, bool online);
    void reportWireless(int signal);
    ConnectionState connectionState() const { return state_; }
    void startConnectivity();
    void stopConnectivity();
    bool isConnected() const { return state_ == ConnectionState::Connected; }
    QString examId() const { return examId_; }

signals:
    void terminationRequested();
    void lockScreenRequested(const QString &message);
    void lockScreenConfirmed();
    void handConfirmed();

private:
    void execute(
        Completion done,
        const QByteArray &method,
        const QString &endpoint,
        const QByteArray &body = {},
        const QByteArray &contentType = {},
        bool authorization = false,
        bool connectionToken = false,
        bool refreshToken = true,
        const QHash<QByteArray, QByteArray> &additionalHeaders = {});
    void requestOAuthToken(Completion done);
    void cancelRequests();
    void clearConnection();
    void flushLogs();
    void sendNotification(const QJsonObject &object, Completion done = {});
    void performPing();
    void handleInstruction(const data::InstructionData &instruction);
    QUrl endpointUrl(const QString &endpoint) const;
    static QByteArray formEncode(const QList<QPair<QString, QString>> &values);
    static QByteArray firstHeader(
        const QHash<QByteArray, QByteArray> &headers,
        const QByteArray &name);

    seb::configuration::contracts::AppConfig appConfig_;
    seb::settings::server::ServerSettings settings_;
    QUrl baseUrl_;
    data::Api api_;
    QString connectionToken_;
    QString oauth2Token_;
    QString examId_;
    QNetworkAccessManager network_;
    QTimer pingTimer_;
    QQueue<QString> instructionConfirmations_;
    int pingNumber_ = 0;
    int notificationId_ = 0;
    int currentHandId_ = 0;
    int pendingHandId_ = 0;
    quint64 handRevision_ = 0;
    int currentLockScreenId_ = 0;
    ConnectionState state_ = ConnectionState::Disconnected;
    QTimer logTimer_;
    QQueue<QJsonObject> logs_;
    QSet<QNetworkReply *> replies_;
    QSet<QTimer *> retryTimers_;
    QList<Completion> tokenWaiters_;
    quint64 generation_ = 0;
    bool refreshingToken_ = false;
    bool logInFlight_ = false;
    int batteryPercentage_ = -1;
    int batteryOnline_ = -1;
    int wirelessSignal_ = -2;
    bool pingInFlight_ = false;
};

}  // namespace seb::server
