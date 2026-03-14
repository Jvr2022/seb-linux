#pragma once

#include <QString>

namespace seb::userinterface::contracts::windows::data {
struct CredentialsDialogResult { bool accepted = false; QString username; QString password; };
}
