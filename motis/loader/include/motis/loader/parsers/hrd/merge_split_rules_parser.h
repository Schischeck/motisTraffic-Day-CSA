#pragma once

#include <cinttypes>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct merge_split_service_rule {
  bool operator==(merge_split_service_rule const& lhs,
                  merge_split_service_rule const& rhs) const;
  bool operator<(merge_split_service_rule const& lhs,
                 merge_split_service_rule const& rhs) const;
  std::pair<int, uint64_t> service_key_1, service_key_2;
  int eva_num_begin, eva_num_end;
  bitfield mask;
};

std::vector<merge_split_service_rule> parse_merge_split_service_rules(
    loaded_file const&, bitfield_translator);

}  // hrd
}  // loader
}  // motis
