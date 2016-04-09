#pragma once

#include <cinttypes>
#include <vector>

#include "motis/loader/hrd/model/service_rule.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_through_service_rules(loaded_file const&,
                                 std::map<int, bitfield> const&,
                                 service_rules&);

}  // hrd
}  // loader
}  // motis
