#pragma once
#include "../../contracts/windows/i_server_failure_dialog.h"
class QWidget;
namespace seb::userinterface::desktop::windows { class ServerFailureDialog : public contracts::windows::IServerFailureDialog { public: explicit ServerFailureDialog(QWidget *parent = nullptr); contracts::windows::data::ServerFailureDialogResult exec() override; private: QWidget *parent_ = nullptr; }; }
