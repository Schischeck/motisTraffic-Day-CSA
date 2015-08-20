#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/hrd/service/shared_data.h"
#include "motis/schedule-format/Service_generated.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_services(loaded_file file, shared_data const& stamm,
                    flatbuffers::FlatBufferBuilder& fbb,
                    std::vector<flatbuffers::Offset<Service>>& services);

}  // hrd
}  // loader
}  // motis
