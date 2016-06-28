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

TEST(routes_preprocessing_osm_extraction, extract_osm_data) {
  std::vector<int64_t> nodes = {100, 101, 102};
  std::vector<int64_t> relations = {0, 1, 2, 3, 4, 5, 6, 7};
  std::string file =
      "modules/routes/test/test_preprocessing/test_osm/simple_graph.osm";
  std::map<int64_t, osm_node> osm_nodes;
  std::map<int64_t, osm_route> osm_routes;
  osm_loader osm_loader(file, osm_nodes, osm_routes);
  osm_loader.load_osm();
  ASSERT_EQ(8, osm_routes.size());
  for (uint32_t i = 0; i < osm_routes.size(); i++) {
    ASSERT_TRUE(osm_routes.count(i) == 1);
    EXPECT_EQ(nodes, osm_routes.at(i).railways_);
    EXPECT_EQ(i, osm_routes.at(i).clasz_);
  }
  ASSERT_EQ(3, osm_nodes.size());
  EXPECT_EQ(relations, osm_nodes.at(100).relations_);
  EXPECT_EQ(relations, osm_nodes.at(101).relations_);
  EXPECT_EQ(relations, osm_nodes.at(102).relations_);
}
