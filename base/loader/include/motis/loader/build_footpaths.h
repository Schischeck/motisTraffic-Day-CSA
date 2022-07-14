#pragma once

#include "motis/core/common/hash_helper.h"
#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"

#include "motis/core/schedule/schedule.h"

#include "motis/loader/loader_options.h"

#include <vector>

namespace motis::loader {

struct Schedule;  // NOLINT
struct Station;  // NOLINT

void build_footpaths(unsigned&, schedule&, bool const&, bool const&,
                     hash_map<Station const*, station_node*> const&,
                     Schedule const* const&);

}  // namespace motis::loader
