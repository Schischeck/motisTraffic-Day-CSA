#pragma once

#include <cinttypes>
#include <map>
#include <string>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;

template <typename T>
std::map<uint64_t, std::string> parse_directions(loaded_file const& file,
                                                 T const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
