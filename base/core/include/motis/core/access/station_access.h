#pragma once

#include <string>

#include "motis/core/access/error.h"
#include "motis/core/schedule/schedule.h"

namespace motis {

inline station* get_station(schedule const& sched, std::string const& eva_nr) {
  auto it = sched.eva_to_station.find(eva_nr);
  if (it == end(sched.eva_to_station)) {
    throw boost::system::system_error(access::error::station_not_found);
  }

  return it->second;
}

}  // namespace motis
