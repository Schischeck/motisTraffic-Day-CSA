#pragma once

#include <cinttypes>
#include <memory>
#include <vector>
#include <map>

#include "boost/optional.hpp"

#include "motis/loader/loaded_file.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

// key := (is_first, train_num, admin, eva_num)
typedef std::tuple<bool, int, uint64_t, int> through_train_key;

// rule := (bitfield number, service1, service2)
typedef std::tuple<int, boost::optional<hrd_service>,
                   boost::optional<hrd_service>> through_train_rule;

typedef std::map<through_train_key,
                 std::vector<std::shared_ptr<through_train_rule>>>
    through_trains_map;

through_trains_map parse_through_train_rules(loaded_file const& src);

}  // hrd
}  // loader
}  // motis
