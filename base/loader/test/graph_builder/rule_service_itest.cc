#include "gtest/gtest.h"

#include "motis/test/motis_instance_test.h"
#include "../hrd/test_spec_test.h"

namespace fbs = flatbuffers;
using namespace motis;
using namespace motis::test;
using namespace motis::module;
using namespace motis::loader;
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
      "station": {
        "name": "",
        "id": "0000002"
      },
      "interval": {
        "begin": 1448323200,
        "end": 1448336800
      }
    },
    "destination": {
      "name": "",
      "id": "0000009"
    },
    "additional_edges": [],
    "via": []
  }
})";

std::vector<int> trip_train_nrs_at(
    int from, int to, fbs::Vector<fbs::Offset<Trip>> const* trips) {
  std::vector<int> train_nrs;
  for (auto const& t : *trips) {
    if (t->range()->from() < to && t->range()->to() >= from) {
      train_nrs.push_back(t->id()->train_nr());
    }
  }
  std::sort(begin(train_nrs), end(train_nrs));
  return train_nrs;
}

struct loader_graph_builder_rule_service : public motis_instance_test {
  loader_graph_builder_rule_service()
      : motis_instance_test(
            {(hrd::SCHEDULES / "mss-ts").generic_string(), "20151124"},
            {"routing"}) {}
};

TEST_F(loader_graph_builder_rule_service, search) {
  auto res = call(make_msg(routing_request));
  auto connections = motis_content(RoutingResponse, res)->connections();

  ASSERT_EQ(1, connections->size());
  for (unsigned i = 0; i < connections->Get(0)->stops()->size() - 2; ++i) {
    EXPECT_FALSE(connections->Get(0)->stops()->Get(i)->interchange());
  }

  auto const trips = connections->Get(0)->trips();
  EXPECT_EQ(std::vector<int>({1, 2, 3, 4}), trip_train_nrs_at(0, 5, trips));
  EXPECT_EQ(std::vector<int>({3}), trip_train_nrs_at(0, 1, trips));
  EXPECT_EQ(std::vector<int>({1, 2, 3}), trip_train_nrs_at(1, 4, trips));
}
