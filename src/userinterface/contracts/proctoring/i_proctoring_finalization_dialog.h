#pragma once
namespace seb::userinterface::contracts::proctoring { class IProctoringFinalizationDialog { public: virtual ~IProctoringFinalizationDialog() = default; virtual int exec() = 0; }; }
