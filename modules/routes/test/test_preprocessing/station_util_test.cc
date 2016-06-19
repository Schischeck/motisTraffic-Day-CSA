#include <iostream>

#include "gtest/gtest.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/station.h"
#include "motis/bootstrap/dataset_settings.h"
#include "motis/bootstrap/motis_instance.h"
#include "motis/loader/loader.h"
#include "motis/routes/preprocessing/osm/osm_loader.h"
#include "motis/routes/preprocessing/osm/osm_node.h"
#include "motis/routes/preprocessing/osm/osm_route.h"
#include "motis/routes/preprocessing/station_matcher.h"
#include "./test_schedule_setup.h"

using namespace motis;
using namespace motis::routes;

namespace motis {
namespace routes {
class routes_preprocessing_util : public test_schedule_setup {
public:
  routes_preprocessing_util()
      : test_schedule_setup(
            "modules/routes/test/test_preprocessing/test_schedule/",
            "20160101") {}
};

TEST_F(routes_preprocessing_util, node_geo_index) {
  node_geo_index geo_index(osm_nodes_);
  std::vector<int64_t> expected_station1 = {100};
  std::vector<int64_t> expected_station2 = {101};
  auto actual = geo_index.nodes(50.15999999999997, 8.312000000000012, 500);
  ASSERT_EQ(expected_station1, actual);
  actual = geo_index.nodes(50.18000000000001, 8.288000000000011, 500);
  ASSERT_EQ(expected_station2, actual);
  actual = geo_index.nodes(50.163, 8.305, 500);
  ASSERT_TRUE(actual.empty());
}

TEST_F(routes_preprocessing_util, min_clasz) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  ASSERT_EQ(9, station_matcher.min_clasz(15));
  ASSERT_EQ(3, station_matcher.min_clasz(12));
  ASSERT_EQ(3, station_matcher.min_clasz(13));
}

TEST_F(routes_preprocessing_util, extract_nodes) {
  std::vector<int64_t> node_section;
  std::vector<int64_t> expected = {100, 101, 102};
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  ASSERT_EQ(expected,
            station_matcher.extract_nodes(osm_routes_.at(1).railways_, 100, 102));
}
} // namespace routes
} // namespace motis
