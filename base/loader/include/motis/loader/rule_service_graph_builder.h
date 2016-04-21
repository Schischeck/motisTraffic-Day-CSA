#include "motis/loader/graph_builder.h"

namespace motis {
namespace loader {

struct rule_service_graph_builder {
  explicit rule_service_graph_builder(graph_builder&);

  void add_rule_services(
      flatbuffers::Vector<flatbuffers::Offset<RuleService>> const*
          rule_services);

private:
  void add_rule_service(RuleService const* r);

  graph_builder& gb_;
  std::map<Route const*, route const*> routes_;
};

}  // namespace loader
}  // namespace motis
