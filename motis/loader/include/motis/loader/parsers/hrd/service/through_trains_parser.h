#pragma once

#include <cinttypes>
#include <memory>
#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct through_train_rules {
  // key := (train_num, admin, eva_num)
  typedef std::tuple<uint64_t, uint64_t, int> key;
  typedef std::tuple<bitfield, hrd_service, hrd_service> rule;

  bool try_apply_rule(hrd_service const& s, std::vector<hrd_service>& result);

  // the following map caches for through trains rules until they are
  // applicable. A rule has one of two states:
  //
  // - incomplete: the rule is not yet applicable since at least one the
  //               involved hrd services has not been parsed yet.
  // - complete  : both hrd services are available and may be processed.
  //
  // Finally, to ensure the consistency of the schedule, all cached hrd service
  // have to be processed, irrespectively whether a through train rule is
  // complete or not.
  //
  // HINTS:
  // 1. 2:1 Mappings
  //
  //    The keys of two hrd_services map to a shared rule object.
  //
  //    EXAMPLE:
  //    (123456, 80____, 8012345)_
  //                              \__(b1, s1, s2)
  //    (654321, 80____, 8012345)_/
  //
  // 2. Caching
  //
  //    A rule should be applied as soon as possible. If applied, the key-value
  //    pair should be removed.
  std::map<key, std::shared_ptr<rule>> through_train_rules_;
};

through_train_rules parse_through_train_rules(loaded_file const& src);

}  // hrd
}  // loader
}  // motis
