#include "gtest/gtest.h"

#include "motis/test/motis_instance_helper.h"
#include "../hrd/test_spec_test.h"

using namespace motis::test;
using namespace motis::module;

auto routing_request = R"(
{
  "content_type": "RoutingRequest",
  "content": {
    "interval": {
      "begin": 1448323200,
      "end": 1448336800
    },
    "type": "PreTrip",
    "direction": "Forward",
    "path": [
      { "eva_nr": "0000002", "name": "" },
      { "eva_nr": "0000009", "name": "" }
    ]
  }
}
)";

namespace motis {
namespace loader {

TEST(loader_graph_builder_rule_service, search) {
  message::init_parser();

  auto instance = launch_motis((hrd::SCHEDULES / "mss-ts").generic_string(),
                               "20151124", {"routing"});

  auto res = send(instance, make_msg(routing_request));
  ASSERT_EQ(MsgContent_RoutingResponse, res->content_type());

  auto const& connections =
      res->content<routing::RoutingResponse const*>()->connections();

  ASSERT_EQ(1, connections->size());
  for (int i = 0; i < connections->Get(0)->stops()->size() - 2; ++i) {
    auto stop = connections->Get(0)->stops()->Get(i);
    EXPECT_FALSE(stop->interchange() ? true : false);
  }
}

}  // namespace loader
}  // namespace motis
