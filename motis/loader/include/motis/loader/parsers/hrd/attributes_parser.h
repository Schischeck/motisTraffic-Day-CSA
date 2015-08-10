#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

#include "parser/cstr.h"

#include "motis/schedule-format/Attribute_generated.h"

namespace motis {
namespace loader {
namespace hrd {

std::vector<flatbuffers::Offset<Attribute>> parse_attributes(
    flatbuffers::FlatBufferBuilder&, parser::cstr file_content);

}  // hrd
}  // loader
}  // motis
