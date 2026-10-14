#pragma once
#include "base_data_mapper.h"
namespace seb::configuration::configuration_data::data_mapping { class ServerDataMapper : public BaseDataMapper { public: void map(const QString &, const QVariant &, seb::settings::AppSettings &) const override; }; }
