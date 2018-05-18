#pragma once

#include <cinttypes>
#include <map>
#include <string>

#include "motis/loader/hrd/parse_config.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<uint16_t, std::string> parse_attributes(loaded_file const&,
                                                 config const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
