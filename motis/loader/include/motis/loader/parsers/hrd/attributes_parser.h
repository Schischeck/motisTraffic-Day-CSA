#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

#include "motis/schedule-format/Attribute_generated.h"

namespace motis {
namespace loader {
namespace hrd {

std::vector<flatbuffers::Offset<Attribute>> parse_attributes(
    flatbuffers::FlatBufferBuilder&, cstr);

}  // hrd
}  // loader
}  // motis
