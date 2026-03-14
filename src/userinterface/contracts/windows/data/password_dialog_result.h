#pragma once

#include <QString>

namespace seb::userinterface::contracts::windows::data {
struct PasswordDialogResult { bool accepted = false; QString password; };
}
