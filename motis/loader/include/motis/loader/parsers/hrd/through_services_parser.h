#pragma once

#include <cinttypes>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct through_service_rule {
  // key := (service number, administration)
  typedef std::pair<int, uint64_t> key;

  bool operator==(through_service_rule const& lhs,
                  through_service_rule const& rhs) const;
  bool operator<(through_service_rule const& lhs,
                 through_service_rule const& rhs) const;

  std::pair<int, uint64_t> service_key_1, service_key_2;
  int eva_num;
  bitfield mask;
};

std::map<through_service_rule::key,
         std::vector<std::shared_ptr<through_service_rule>>>
parse_through_service_rules(loaded_file const&, bitfield_translator);

}  // hrd
}  // loader
}  // motis
