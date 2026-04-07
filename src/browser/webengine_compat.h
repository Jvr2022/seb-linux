#pragma once

#ifndef SEB_HAS_QTWEBENGINE
#define SEB_HAS_QTWEBENGINE 0
#endif

namespace seb::browser {

inline constexpr bool kHasQtWebEngine =
#if SEB_HAS_QTWEBENGINE
    true;
#else
    false;
#endif

}  // namespace seb::browser
