#include "motis/routes/preproc/db/railway_finder.h"

// #include "parser/file.h"

// #include "motis/protocol/RoutesSection_generated.h"
// #include "motis/protocol/RoutesSections_generated.h"

namespace motis {
namespace routes {

// railway_finder::railway_finder(schedule const& sched, railway_graph& graph)
//     : sched_(sched), graph_(graph){};

// void railway_finder::find_railways(std::string file) {
//   std::vector<flatbuffers::Offset<RoutesSection>> railway_sections;
//   flatbuffers::FlatBufferBuilder builder;
//   for (auto const& node : sched_.station_nodes_) {
//     for (auto const& id : get_connected_stations(*node)) {
//       railway_sections.push_back(CreateRoutesSection(
//           builder, node->id_, id, 0, builder.CreateVector(shortest_path(
//                                          *node, *sched_.station_nodes_[id]))));
//     }
//   }

//   auto railway_fb =
//       CreateRoutesSections(builder, builder.CreateVector(railway_sections));
//   builder.Finish(railway_fb);
//   parser::file(file.c_str(), "w+")
//       .write(builder.GetBufferPointer(), builder.GetSize());
// }

// std::vector<double> railway_finder::shortest_path(station_node const& start,
//                                                   station_node const& end) {
//   std::vector<double> coords;
//   return coords;
// }

// std::vector<int> railway_finder::get_connected_stations(
//     station_node const& node) {
//   std::vector<int> visited;
//   std::vector<int> result;
//   for (auto const& route_node : node.get_route_nodes()) {
//     for (auto const& edge : route_node->edges_) {
//       if (edge.empty()) {
//         continue;
//       }
//       auto const& dest = edge.get_destination()->get_station();
//       auto const& clasz = edge.m_.route_edge_.conns_[0].full_con_->clasz_;
//       if (clasz == 8 ||
//           std::find_if(result.begin(), result.end(), [dest](int a) -> bool {
//             return a == dest->id_;
//           }) != visited.end()) {
//         continue;
//       }
//       result.push_back(dest->id_);
//     }
//   }
//   return result;
// }

}  // namespace routes
}  // namespace motis
