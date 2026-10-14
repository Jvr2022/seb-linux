#include "data_mapper.h"

#include "data_mapping/application_data_mapper.h"
#include "data_mapping/audio_data_mapper.h"
#include "data_mapping/browser_data_mapper.h"
#include "data_mapping/configuration_file_data_mapper.h"
#include "data_mapping/display_data_mapper.h"
#include "data_mapping/general_data_mapper.h"
#include "data_mapping/input_data_mapper.h"
#include "data_mapping/proctoring_data_mapper.h"
#include "data_mapping/security_data_mapper.h"
#include "data_mapping/server_data_mapper.h"
#include "data_mapping/service_data_mapper.h"
#include "data_mapping/system_data_mapper.h"
#include "data_mapping/user_interface_data_mapper.h"

#include <array>
#include <memory>

namespace seb::configuration::configuration_data {

class DataMapper::MapperCollection
{
public:
    std::array<std::unique_ptr<data_mapping::BaseDataMapper>, 13> values;
};

DataMapper::DataMapper()
    : mappers_(std::make_unique<MapperCollection>())
{
    mappers_->values = {
        std::make_unique<data_mapping::ApplicationDataMapper>(),
        std::make_unique<data_mapping::AudioDataMapper>(),
        std::make_unique<data_mapping::BrowserDataMapper>(),
        std::make_unique<data_mapping::ConfigurationFileDataMapper>(),
        std::make_unique<data_mapping::DisplayDataMapper>(),
        std::make_unique<data_mapping::GeneralDataMapper>(),
        std::make_unique<data_mapping::InputDataMapper>(),
        std::make_unique<data_mapping::ProctoringDataMapper>(),
        std::make_unique<data_mapping::SecurityDataMapper>(),
        std::make_unique<data_mapping::ServerDataMapper>(),
        std::make_unique<data_mapping::ServiceDataMapper>(),
        std::make_unique<data_mapping::SystemDataMapper>(),
        std::make_unique<data_mapping::UserInterfaceDataMapper>()
    };
}

DataMapper::~DataMapper() = default;

void DataMapper::map(const QVariantMap &data, seb::settings::AppSettings &settings) const
{
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        for (const auto &mapper : mappers_->values) {
            mapper->map(it.key(), it.value(), settings);
        }
    }
    for (const auto &mapper : mappers_->values) {
        mapper->mapGlobal(data, settings);
    }
}

}  // namespace seb::configuration::configuration_data
