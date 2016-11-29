#include "gtest/gtest.h"

#include <set>

#include "boost/filesystem.hpp"

#include "parser/file.h"

#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

#include "motis/routes/db/mapdb.h"
#include "motis/routes/lookup_index.h"
#include "motis/routes/routes.h"

#include "motis/routes/prepare/prepare_data.h"
#include "motis/routes/prepare/station_sequences.h"

#include "motis/protocol/RoutesSeqResponse_generated.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
namespace fs = boost::filesystem;
using motis::test::schedule::simple_realtime::dataset_opt;

namespace motis {
namespace routes {

constexpr auto kRoutesIndexRequest = R""(
{
  "destination": {
    "type": "Module",
    "target": "/routes/index"
  },
  "content_type": "RoutesIndexRequest",
  "content": {
    "index": "0"
  }
}
)"";

constexpr auto kRoutesIdTrainRequest = R""(
{
  "destination": {
    "type": "Module",
    "target": "/routes/id_train"
  },
  "content_type": "RoutesIdTrainRequest",
  "content": {
    "trip_id": {
      "station_id": "",
      "train_nr": 628,
      "time": 1448362440,
      "target_station_id": "",
      "target_time": 1448382360,
      "line_id": ""
    }
  }
}
)"";

constexpr auto kRoutesStationSeqRequest = R""(
{
  "destination": {
    "type": "Module",
    "target": "/routes/station_seq"
  },
  "content_type": "RoutesStationSeqRequest",
  "content": {
    "station_ids": [
      "0", "1"
    ],
    "clasz": 0
  }
}
)"";

struct routes_integration_test : public motis_instance_test {
  routes_integration_test()
      : motis_instance_test(dataset_opt, {"routes"}, {"--routes.db=:memory:"}),
        db_("") {
    station_seq seq;
    seq.station_ids_.emplace_back("0");
    seq.station_ids_.emplace_back("1");
    seq.coordinates_.emplace_back(48.140232, 11.558335);
    seq.coordinates_.emplace_back(49.445616, 11.082989);
    seq.categories_.emplace(0);

    prepare_data data;
    data.sequences_.emplace_back(std::move(seq));
    strategies routing_strategies;
    auto stub = std::make_unique<stub_routing>(0);
    routing_strategies.strategies_.push_back(std::move(stub));
    prepare(data, routing_strategies, db_, osm_);
    auto& routes_module = get_routes_module();
    routes_module.db_ = std::make_unique<map_database>(db_);
    routes_module.lookup_ =
        std::make_unique<lookup_index>(routes_module.db_->get("__index"));
  }

  routes& get_routes_module() { return get_module<routes>("routes"); }

  map_database db_;
  std::string osm_ = "./modules/routes/test_resources/test_osm.xml";
};

TEST_F(routes_integration_test, no_duplicates) {
  std::set<std::string> msgs;
  ASSERT_TRUE(db_.db_.size() > 0);
  for (auto i = 0u; i < db_.db_.size() - 1; ++i) {
    auto const& buf = db_.get(std::to_string(i));
    auto m = std::make_shared<message>(buf.size(), buf.c_str());
    auto result = msgs.insert(m->to_json());
    ASSERT_TRUE(result.second);
  }
}

TEST_F(routes_integration_test, correct_index) {
  ASSERT_EQ(
      1, get_routes_module().lookup_->lookup_table_.get()->indices()->size());
}

TEST_F(routes_integration_test, prepare_index) {
  std::vector<double> expected = {48.140232, 11.558335, 48.140234, 11.558336,
                                  49.445615, 11.082988, 49.445616, 11.082989};

  auto msg = call(make_msg(kRoutesIndexRequest));
  auto resp = motis_content(RoutesSeqResponse, msg);

  ASSERT_EQ(2, resp->station_ids()->size());
  ASSERT_EQ(1, resp->classes()->size());
  ASSERT_EQ(1, resp->segments()->size());

  auto const& polyline = resp->segments()->Get(0)->coordinates();
  ASSERT_EQ(8, polyline->size());
  for (auto i = 0u; i < polyline->size(); ++i) {
    ASSERT_EQ(expected[i], polyline->Get(i));
  }
}

TEST_F(routes_integration_test, prepare_station_seq) {
  std::vector<double> expected = {48.140232, 11.558335, 48.140234, 11.558336,
                                  49.445615, 11.082988, 49.445616, 11.082989};

  auto msg = call(make_msg(kRoutesStationSeqRequest));
  auto resp = motis_content(RoutesSeqResponse, msg);

  ASSERT_EQ(2, resp->station_ids()->size());
  ASSERT_EQ(1, resp->classes()->size());
  ASSERT_EQ(1, resp->segments()->size());

  auto const& polyline = resp->segments()->Get(0)->coordinates();
  ASSERT_EQ(8, polyline->size());
  for (auto i = 0u; i < polyline->size(); ++i) {
    ASSERT_EQ(expected[i], polyline->Get(i));
  }
}

// TODO: use real schedule and make a id_train request
TEST_F(routes_integration_test, DISABLED_prepare_id_train) {
  std::vector<double> expected = {48.140232, 11.558335, 48.140234, 11.558336,
                                  49.445615, 11.082988, 49.445616, 11.082989};

  auto msg = call(make_msg(kRoutesIdTrainRequest));
  auto resp = motis_content(RoutesSeqResponse, msg);

  ASSERT_EQ(2, resp->station_ids()->size());
  ASSERT_EQ(1, resp->classes()->size());
  ASSERT_EQ(1, resp->segments()->size());

  auto const& polyline = resp->segments()->Get(0)->coordinates();
  ASSERT_EQ(8, polyline->size());
  for (auto i = 0u; i < polyline->size(); ++i) {
    ASSERT_EQ(expected[i], polyline->Get(i));
  }
}

}  // namespace routes
}  // namespace motis