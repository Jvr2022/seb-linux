#pragma once
namespace seb::settings::security {
enum class ClipboardPolicy {
    Allow,
    Block,
    Isolated,
    Disabled = Block,
    System = Allow
};
}
