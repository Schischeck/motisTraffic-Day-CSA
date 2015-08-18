#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/schedule-format/Service_generated.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_services(
    loaded_file, std::map<int, flatbuffers::Offset<Station>> const& stations,
    std::map<uint16_t, flatbuffers::Offset<Attribute>> const& attributes,
    std::map<int, flatbuffers::Offset<flatbuffers::String>> const& bitfields,
    platform_rules const&, flatbuffers::FlatBufferBuilder&,
    std::vector<flatbuffers::Offset<Service>>& trains);

}  // hrd
}  // loader
}  // motis
