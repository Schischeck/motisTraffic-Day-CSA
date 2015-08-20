#pragma once

#include <cinttypes>
#include <string>
#include <map>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<uint16_t, std::string> parse_attributes(loaded_file);

}  // hrd
}  // loader
}  // motis
