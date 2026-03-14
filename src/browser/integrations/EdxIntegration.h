#pragma once

#include "Integration.h"

namespace seb::browser::integrations {

class EdxIntegration : public Integration
{
public:
    QString name() const override { return QStringLiteral("EdxIntegration"); }
};

}  // namespace seb::browser::integrations
