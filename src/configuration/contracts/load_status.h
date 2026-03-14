#pragma once

namespace seb::configuration::contracts {

enum class LoadStatus
{
    InvalidData,
    LoadWithBrowser,
    NotSupported,
    PasswordNeeded,
    Success,
    UnexpectedError
};

}  // namespace seb::configuration::contracts
