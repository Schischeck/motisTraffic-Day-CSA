#pragma once

#include <string>

namespace motis {

struct schedule;

void write_station_graph(schedule const& sched, std::string const& prefix);

}  // namespace motis
