#ifndef TD_STATION_GRAPH_H_
#define TD_STATION_GRAPH_H_

#include <string>

namespace td {

struct schedule;

void write_station_graph(schedule const& sched, std::string const& prefix);

}  // namespace td

#endif  // TD_STATION_GRAPH_H_
