#pragma once

#include <string>

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

void find_perfect_matches(motis::loader::Schedule const* sched,
                          std::string const& osm_file);

}  // namespace routes
}  // namespace motis
