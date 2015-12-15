#include "motis/loader/graph_builder.h"

namespace motis {
namespace loader {

struct rule_service_graph_builder {
  rule_service_graph_builder(graph_builder&);

  void add_rule_services(
      flatbuffers::Vector<flatbuffers::Offset<RuleService>> const*
          rule_services);

  route const* add_service(Service const* service, int route_index = -1);

  route const* add_remaining_merge_split_sections(
      bitfield const& traffic_days, int route_index, Service const* new_service,
      Rule const* r, route const* existing_service_route_nodes);

  std::pair<route_info, edge*> add_route_section(
      int route_index, Station const* from_stop, bool in_allowed,
      bool out_allowed, edge* last_route_edge, bool build_outgoing_route_edge,
      node* route_node = nullptr);

  graph_builder& gb_;
  std::map<Route const*, route const*> routes_;
  std::vector<std::unique_ptr<route>> routes_mem_;
};

}  // loader
}  // motis
