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

struct InstructionData
{
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
