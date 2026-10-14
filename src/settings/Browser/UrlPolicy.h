#pragma once
namespace seb::settings::browser {
enum class UrlPolicy {
    Never,
    LoadError,
    BeforeTitle,
    Always,
    SameWindow = BeforeTitle,
    NewWindow = Always,
    NeverOpen = Never,
    OpenInSameWindow = SameWindow,
    OpenInNewWindow = NewWindow
};
}
