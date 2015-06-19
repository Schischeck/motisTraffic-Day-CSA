#include "motis/prep/station_graph.h"

#include <ostream>
#include <fstream>

#include "motis/core/schedule/schedule.h"
#include "motis/loader/files.h"

namespace td {

typedef std::pair<int /* to node id */, int /* train class */> station_graph_edge;

std::vector<station_graph_edge> station_graph_edges(station_node const* node)
{
  std::vector<station_graph_edge> edges;

  for (auto const& edge : node->_edges)
  {
    // destination can be:
    // - a route node of this station
    // - the foot node of this station
    auto const* edge_dest = edge.get_destination();

    // destination of edges for route/foot node can be:
    // - for route nodes: back to station node (this station)
    // - for route nodes: route edge to another station
    // - for foot nodes: station node of another station
    for (auto const& edge_dest_edge : edge_dest->_edges)
    {
      // skip foot edges and empty route edges.
      if (edge_dest_edge.empty())
        continue;

      // for a route edge:
      edges.emplace_back(
          edge_dest_edge.get_destination()->get_station()->_id,
          edge_dest_edge._m._route_edge._conns[0]._full_con->clasz);
    }
  }

  std::sort(std::begin(edges), std::end(edges));
  edges.erase(
      std::unique(std::begin(edges), std::end(edges)),
      std::end(edges));

  return edges;
}

void write_station_graph(schedule const& sched, std::string const& prefix)
{
  std::ofstream out(prefix + STATION_GRAPH);
  out.exceptions(std::ios_base::failbit);

  for (auto& station_node : sched.station_nodes)
  {
    auto const& from_eva = sched.stations[station_node->_id]->eva_nr;
    for (auto& station_graph_edge : station_graph_edges(station_node.get()))
    {
      int to_node_id, clasz;
      std::tie(to_node_id, clasz) = station_graph_edge;
      auto const& to_eva = sched.stations[to_node_id]->eva_nr;
      out << from_eva << ";" << to_eva << ";" << clasz << ";\n";
    }
  }
}

}  // namespace td

