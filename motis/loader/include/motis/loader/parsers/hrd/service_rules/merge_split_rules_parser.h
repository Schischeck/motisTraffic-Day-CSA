#pragma once

#include <cinttypes>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_merge_split_service_rules(
    loaded_file const&, std::map<int, bitfield> const& hrd_bitfields, rules&);

}  // hrd
}  // loader
}  // motis
