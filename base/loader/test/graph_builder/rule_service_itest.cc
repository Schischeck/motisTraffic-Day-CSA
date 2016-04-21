#include "gtest/gtest.h"

#include "../hrd/test_spec_test.h"
#include "motis/test/motis_instance_helper.h"

using namespace motis::test;
using namespace motis::module;
using motis::routing::RoutingResponse;

auto routing_request = R"(
{
  "destination": {
    "type": "Module",
    "target": "/routing"
  },
  "content_type": "RoutingRequest",
  "content": {
    "interval": {
      "begin": 1448323200,
      "end": 1448336800
    },
    "type": "PreTrip",
    "direction": "Forward",
    "path": [
      { "element_type": "StationPathElement",
        "element": {
          "eva_nr": "0000002",
          "name": ""
        }
      }, // Würzburg
      { "element_type": "StationPathElement",
        "element": {
          "eva_nr": "0000009",
          "name": ""
        }
      } // Köln-Ehrenfeld
    ],
    additional_edges: []
  }
}
)";

namespace motis {
namespace loader {

TEST(loader_graph_builder_rule_service, search) {
  auto instance = launch_motis((hrd::SCHEDULES / "mss-ts").generic_string(),
                               "20151124", {"routing"});

  auto res = call(instance, make_msg(routing_request));
  auto connections = motis_content(RoutingResponse, res)->connections();

  ASSERT_EQ(1, connections->size());
  for (unsigned i = 0; i < connections->Get(0)->stops()->size() - 2; ++i) {
    EXPECT_FALSE(connections->Get(0)->stops()->Get(i)->interchange());
  }
}

}  // namespace loader
}  // namespace motis
