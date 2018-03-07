#pragma once

#include <cinttypes>
#include <map>
#include <string>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

template <typename T>
std::map<uint16_t, std::string> parse_attributes(loaded_file const&, T const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
