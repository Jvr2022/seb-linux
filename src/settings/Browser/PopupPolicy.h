#pragma once
namespace seb::settings::browser {
enum class PopupPolicy {
    Allow,
    AllowSameHost,
    AllowSameHostAndWindow,
    AllowSameWindow,
    Block,
    SameWindow = AllowSameWindow,
    SameHost = AllowSameHost,
    All = Allow
};
}
