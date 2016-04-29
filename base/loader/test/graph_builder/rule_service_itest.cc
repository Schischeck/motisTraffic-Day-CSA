#include "gtest/gtest.h"

#include "motis/test/motis_instance_helper.h"
#include "../hrd/test_spec_test.h"

using namespace motis::test;
using namespace motis::module;
using motis::routing::RoutingResponse;

auto routing_request = R"({
  "destination": {
    "type": "Module",
    "target": "/routing"
  },
  "content_type": "RoutingRequest",
  "content": {
    "start_type": "PretripStart",
    "start": {
      "station_id": "0000002",
      "interval": {
        "begin": 1448323200,
        "end": 1448336800
      }
    },
    "path": [ "0000009" ],
    "additional_edges": []
  }
})";

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
