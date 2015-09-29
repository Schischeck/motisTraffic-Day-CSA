#pragma once

#include <cinttypes>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct through_service_rule {
  std::pair<int, uint64_t> service_key_from, service_key_to;
  int eva_num, bitfield_num;
};

std::vector<through_service_rule> parse_through_service_rules(
    loaded_file const& src);

}  // hrd
}  // loader
}  // motis
