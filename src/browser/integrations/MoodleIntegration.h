#pragma once

#include "Integration.h"

namespace seb::browser::integrations {

class MoodleIntegration : public Integration
{
public:
    QString name() const override { return QStringLiteral("MoodleIntegration"); }
};

}  // namespace seb::browser::integrations
