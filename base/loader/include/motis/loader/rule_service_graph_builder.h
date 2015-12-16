#include "motis/loader/graph_builder.h"

namespace motis {
namespace loader {

struct rule_service_graph_builder {
  rule_service_graph_builder(graph_builder&);

  void add_rule_services(
      flatbuffers::Vector<flatbuffers::Offset<RuleService>> const*
          rule_services);

  route const* add_service(Service const* service, bitfield const& traffic_days,
                           int first_day, int last_day, int route_index);

  route const* add_remaining_merge_split_sections(
      bitfield const& traffic_days, int first_day, int last_day,
      int route_index, Service const* new_service, Rule const* r,
      route const* existing_service_route_nodes);

  graph_builder& gb_;
  std::map<Route const*, route const*> routes_;
  std::vector<std::unique_ptr<route>> routes_mem_;
};

}  // loader
}  // motis
