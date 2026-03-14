#pragma once

#include "Integration.h"

namespace seb::browser::integrations {

class GenericIntegration : public Integration
{
public:
    QString name() const override { return QStringLiteral("GenericIntegration"); }
};

}  // namespace seb::browser::integrations
