#pragma once

namespace seb::configuration::contracts {

enum class SaveStatus
{
    InvalidData,
    NotSupported,
    Success,
    UnexpectedError
};

}  // namespace seb::configuration::contracts
