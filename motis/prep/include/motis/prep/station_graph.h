#pragma once

#include <string>

namespace td {

struct schedule;

void write_station_graph(schedule const& sched, std::string const& prefix);

}  // namespace td
