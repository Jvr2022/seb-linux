#pragma once

#include "contracts/server_response.h"
#include "data/api.h"
#include "data/connection_info.h"
#include "data/exam.h"
#include "data/instructions.h"

#include "configuration/contracts/app_config.h"
#include "settings/Server/ServerSettings.h"

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
    bool initializeConnection(
        const QString &api,
        const QString &connectionToken,
        const QString &examId,
        const QString &oauth2Token,
        const seb::settings::server::ServerSettings &settings,
        QString *error = nullptr);

    contracts::ServerResponse connectToServer();
    contracts::ServerResponse disconnectFromServer();
    contracts::ServerResponse availableExams(
        QList<data::Exam> &exams,
        const QString &examId = {});
    contracts::ServerResponse downloadExamConfiguration(
        const data::Exam &exam,
        const QString &path);
    contracts::ServerResponse selectExam(
        const data::Exam &exam,
        QString *browserExamKey = nullptr);

    contracts::ServerResponse sendUserIdentifier(const QString &identifier);
    contracts::ServerResponse raiseHand(const QString &message = {});
    contracts::ServerResponse lowerHand();
    contracts::ServerResponse lockScreen(const QString &message = {});
    contracts::ServerResponse confirmLockScreen();

    void startConnectivity();
    void stopConnectivity();
    bool isConnected() const { return connected_; }
    data::ConnectionInfo connectionInfo() const;
    QString examId() const { return examId_; }

signals:
    void terminationRequested();
    void lockScreenRequested(const QString &message);
    void lockScreenConfirmed();
    void handConfirmed();
    void proctoringInstructionReceived(const QString &instruction);
    void proctoringConfigurationReceived(bool allowChat, bool receiveAudio, bool receiveVideo);

private:
    contracts::ServerResponse execute(
        const QByteArray &method,
        const QString &endpoint,
        const QByteArray &body = {},
        const QByteArray &contentType = {},
        bool authorization = false,
        bool connectionToken = false,
        bool refreshToken = true,
        const QHash<QByteArray, QByteArray> &additionalHeaders = {});
    bool requestOAuthToken(QString *error = nullptr);
    contracts::ServerResponse finishHandshake();
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
    int currentLockScreenId_ = 0;
    bool connected_ = false;
    bool pingInFlight_ = false;
};

}  // namespace seb::server
