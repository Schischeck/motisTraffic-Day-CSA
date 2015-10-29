#pragma once

#include <cinttypes>
#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/loader/bitfield.h"

#include "motis/loader/model/hrd/service_rule.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_merge_split_service_rules(loaded_file const&,
                                     std::map<int, bitfield> const&,
                                     service_rules&);

}  // hrd
}  // loader
}  // motis
