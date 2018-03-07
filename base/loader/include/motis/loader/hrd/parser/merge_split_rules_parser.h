#pragma once

#include <cinttypes>
#include <map>

#include "motis/loader/bitfield.h"
#include "motis/loader/loaded_file.h"

#include "motis/loader/hrd/model/service_rule.h"

namespace motis {
namespace loader {
namespace hrd {

template <typename T>
void parse_merge_split_service_rules(loaded_file const&,
                                     std::map<int, bitfield> const&,
                                     service_rules&, T const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
