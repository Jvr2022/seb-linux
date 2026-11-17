#pragma once

#include <QString>

namespace seb::server::data {

enum class Instruction
{
    None,
    LockScreen,
    NotificationConfirm,
    Proctoring,
    ProctoringReconfiguration,
    Quit
};

enum class NotificationType
{
    Unknown,
    LockScreen,
    Hand
};

enum class ProctoringProvider { Unknown, JitsiMeet, Zoom, ScreenProctoring };
enum class ProctoringMethod { Unknown, Join, Leave };
struct ProctoringInstruction
{
    ProctoringProvider provider = ProctoringProvider::Unknown;
    ProctoringMethod method = ProctoringMethod::Unknown;
    QString clientId, clientSecret, groupId, serviceUrl, sessionId, encryptionSecret;
    QString roomName, token;
    QString meetingNumber, password, sdkKey, signature, subject, userName;
};

struct InstructionData
{
    ProctoringInstruction proctoring;
    Instruction instruction = Instruction::None;
    NotificationType notificationType = NotificationType::Unknown;
    QString message;
    QString confirmation;
    int notificationId = 0;
    bool allowChat = false;
    bool receiveAudio = false;
    bool receiveVideo = false;
};

}  // namespace seb::server::data
