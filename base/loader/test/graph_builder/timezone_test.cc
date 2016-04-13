#include "gtest/gtest.h"

#include <tuple>

#include "motis/core/common/date_time_util.h"

#include "motis/loader/util.h"

#include "./graph_builder_test.h"

namespace motis {
namespace loader {

class loader_graph_builder_east_to_west_test
    : public loader_graph_builder_test {
public:
  loader_graph_builder_east_to_west_test()
      : loader_graph_builder_test("east-to-west", to_unix_time(2015, 7, 2),
                                  to_unix_time(2015, 7, 10)) {}
};

void test_events(
    std::tuple<light_connection const*, node const*, node const*> c,
    time expected_dep, time expected_arr) {
  EXPECT_EQ(expected_dep, std::get<0>(c)->d_time_);
  EXPECT_EQ(expected_arr, std::get<0>(c)->a_time_);
}

TEST_F(loader_graph_builder_east_to_west_test, event_times) {
  // Get route starting at Moskva Belorusskaja
  auto node_it = std::find_if(
      begin(sched_->route_index_to_first_route_node_),
      end(sched_->route_index_to_first_route_node_), [&](node const* n) {
        return sched_->stations_[n->get_station()->id_]->eva_nr_ == "2000058";
      });
  auto cs = get_connections(*node_it, 0);
  ASSERT_EQ(23, cs.size());
  test_events(cs[0], motis_time(1630, 0, 180), motis_time(1901, 0, 180));
  // GMT+3 -> GMT+1 (season time)
  test_events(cs[8], motis_time(3106, 0, 180), motis_time(3018, 0, 120));
  test_events(cs[22], motis_time(5306, 0, 120), motis_time(5716, 0, 120));
}

class loader_graph_builder_season_valid : public loader_graph_builder_test {
public:
  loader_graph_builder_season_valid()
      : loader_graph_builder_test("season-valid", to_unix_time(2015, 3, 29),
                                  to_unix_time(2015, 3, 31)) {}
};

TEST_F(loader_graph_builder_season_valid, event_times) {
  // Get route starting at Dortmund Hbf
  auto node_it = std::find_if(
      begin(sched_->route_index_to_first_route_node_),
      end(sched_->route_index_to_first_route_node_), [&](node const* n) {
        return sched_->stations_[n->get_station()->id_]->eva_nr_ == "8000080";
      });
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node_));

  auto cs = get_connections(*node_it, 0);
  ASSERT_EQ(38, cs.size());

  test_events(cs[0], motis_time(53, 0, 60), motis_time(55, 0, 60));
  // +1 -> +2
  test_events(cs[20], motis_time(158, 0, 60), motis_time(300, 0, 120));
  test_events(cs[37], motis_time(344, 0, 120), motis_time(347, 0, 120));
}

class loader_graph_builder_season_invalid : public loader_graph_builder_test {
public:
  loader_graph_builder_season_invalid()
      : loader_graph_builder_test("season-invalid", to_unix_time(2015, 3, 29),
                                  to_unix_time(2015, 3, 31)) {}
};

TEST_F(loader_graph_builder_season_invalid, event_times) {
  // Get route starting at Muenster(Westf)Hbf
  auto node_it = std::find_if(
      begin(sched_->route_index_to_first_route_node_),
      end(sched_->route_index_to_first_route_node_), [&](node const* n) {
        return sched_->stations_[n->get_station()->id_]->eva_nr_ == "8000263";
      });
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node_));

  auto cs = get_connections(*node_it, 0);
  ASSERT_EQ(10, cs.size());

  test_events(cs[0], motis_time(108, 0, 60), motis_time(111, 0, 60));
  // +1 -> +2
  test_events(cs[9], motis_time(154, 0, 60), motis_time(204, 0, 120 - 60));
}

}  // namespace loader
}  // namespace motis
