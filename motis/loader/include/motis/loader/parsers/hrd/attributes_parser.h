#pragma once

#include <cinttypes>
#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Attribute_generated.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<uint16_t, flatbuffers::Offset<Attribute>> parse_attributes(
    loaded_file, flatbuffers::FlatBufferBuilder&);

}  // hrd
}  // loader
}  // motis
