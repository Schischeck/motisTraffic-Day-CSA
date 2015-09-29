#pragma once

#include <cinttypes>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct merge_split_rule {
  std::pair<int, uint64_t> service_key_1, service_key_2;
  int eva_num_begin, eva_num_end, bitfield_num;
};

std::vector<merge_split_rule> parse_merge_split_rules(loaded_file const& src);

}  // hrd
}  // loader
}  // motis
