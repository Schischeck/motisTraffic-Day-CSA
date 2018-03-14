#pragma once

#include <cinttypes>
#include <map>
#include <string>

#include "motis/loader/hrd/parse_config_inheritance.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;

std::map<uint64_t, std::string> parse_directions(loaded_file const& file,
                                                 config const&);
}  // namespace hrd
}  // namespace loader
}  // namespace motis
