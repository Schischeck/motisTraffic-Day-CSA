#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Attribute_generated.h"

namespace motis {
namespace loader {
namespace hrd {

std::vector<flatbuffers::Offset<Attribute>> parse_attributes(
    loaded_file, flatbuffers::FlatBufferBuilder&);

}  // hrd
}  // loader
}  // motis
