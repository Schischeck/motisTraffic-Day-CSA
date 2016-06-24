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
class routes_preprocessing_test : public test_schedule_setup {
public:
  routes_preprocessing_test()
      : test_schedule_setup(
            "modules/routes/test/test_preprocessing/test_schedule2/",
            "20160101") {}
};

TEST_F(routes_preprocessing_test, station_matcher_station_rels) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);

  EXPECT_EQ(8, station_matcher.station_rels_[14].size());  // Station 3
  EXPECT_EQ(8, station_matcher.station_rels_[15].size());  // Bus Station 1
  EXPECT_EQ(0, station_matcher.station_rels_[18].size());  // Station 5
  EXPECT_EQ(0, station_matcher.station_rels_[17].size());  // Station 4
  EXPECT_EQ(0, station_matcher.station_rels_[16].size());  // Bus Station 2
  EXPECT_EQ(8, station_matcher.station_rels_[12].size());  // Station 1
  EXPECT_EQ(8, station_matcher.station_rels_[13].size());  // Station 2

  ASSERT_EQ(100, station_matcher.station_rels_[12][0].second);
  ASSERT_EQ(101, station_matcher.station_rels_[13][0].second);
};

TEST_F(routes_preprocessing_test, match_intersect_both_osm_nodes) {
  std::vector<int64_t> expected_station12 = {100, 101};
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  auto actual_station12 = station_matcher.intersect(12, 13, 0);
  ASSERT_EQ(expected_station12, actual_station12);
};

TEST_F(routes_preprocessing_test, match_intersect_one_with_nodes) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  auto actual_station14 = station_matcher.intersect(12, 17, 0);
  ASSERT_TRUE(actual_station14.empty());
};

TEST_F(routes_preprocessing_test, match_intersect_without_nodes) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  auto actual_station45 = station_matcher.intersect(17, 18, 0);
  ASSERT_TRUE(actual_station45.empty());
};

TEST_F(routes_preprocessing_test, check_node_no_osm_nodes) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  std::vector<double> nodes = {
      schedule_->stations_[17]->width_, schedule_->stations_[17]->length_,
      schedule_->stations_[18]->width_, schedule_->stations_[18]->length_};
  auto actual = station_matcher.find_routes(*schedule_->station_nodes_[17]);
  ASSERT_TRUE(actual.size() == 1);
  ASSERT_EQ(3, std::get<1>(actual[0]));
  ASSERT_EQ(nodes, std::get<2>(actual[0]));
};

TEST_F(routes_preprocessing_test, check_node_no_destination_node) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  std::vector<double> nodes = {
      schedule_->stations_[12]->width_, schedule_->stations_[12]->length_,
      schedule_->stations_[17]->width_, schedule_->stations_[17]->length_};
  std::vector<double> osm_nodes = {};
  auto actual = station_matcher.find_routes(*schedule_->station_nodes_[12]);
  ASSERT_TRUE(actual.size() == 2);
  ASSERT_EQ(3, std::get<1>(actual[1]));
  ASSERT_EQ(nodes, std::get<2>(actual[1]));
};

TEST_F(routes_preprocessing_test, check_node_both_with_osm_nodes) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  auto node1 = osm_nodes_.at(101);
  auto node2 = osm_nodes_.at(102);
  std::vector<double> osm_nodes = {node1.location_.lat(), node1.location_.lon(),
                                   node2.location_.lat(),
                                   node2.location_.lon()};
  auto actual = station_matcher.find_routes(*schedule_->station_nodes_[13]);
  ASSERT_TRUE(actual.size() == 1);
  ASSERT_EQ(3, std::get<1>(actual[0]));
  ASSERT_EQ(osm_nodes, std::get<2>(actual[0]));
};

TEST_F(routes_preprocessing_test, check_node_no_departure_node) {
  station_matcher station_matcher(osm_nodes_, osm_routes_, *schedule_);
  std::vector<double> nodes = {
      schedule_->stations_[17]->width_, schedule_->stations_[17]->length_,
      schedule_->stations_[18]->width_, schedule_->stations_[18]->length_};
  auto actual = station_matcher.find_routes(*schedule_->station_nodes_[17]);
  ASSERT_TRUE(actual.size() == 1);
  ASSERT_EQ(3, std::get<1>(actual[0]));
  ASSERT_EQ(nodes, std::get<2>(actual[0]));
};
} // namespace routes
} // namespace motis
