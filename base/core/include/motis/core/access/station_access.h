#pragma once

#include <string>

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/error.h"

namespace motis {

inline station* get_station(schedule const& sched, std::string const& eva_nr) {
  auto it = sched.eva_to_station_.find(eva_nr);
  if (it == end(sched.eva_to_station_)) {
    throw boost::system::system_error(access::error::station_not_found);
  }
  return it->second;
}

}  // namespace motis
