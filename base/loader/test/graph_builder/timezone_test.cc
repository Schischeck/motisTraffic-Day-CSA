#include "gtest/gtest.h"

#include "./graph_builder_test.h"

#include <tuple>

#include "motis/core/common/date_util.h"

#include "motis/loader/util.h"

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
  EXPECT_EQ(expected_dep, std::get<0>(c)->d_time);
  EXPECT_EQ(expected_arr, std::get<0>(c)->a_time);
}

time exp_time(int day_idx, int hhmm, int offset) {
  return (day_idx + SCHEDULE_OFFSET_MINUTES) + hhmm_to_min(hhmm) - offset;
}

TEST_F(loader_graph_builder_east_to_west_test, event_times) {
  // Get route starting at Moskva Belorusskaja
  auto node_it = std::find_if(
      begin(sched_->route_index_to_first_route_node),
      end(sched_->route_index_to_first_route_node), [&](node const* n) {
        return sched_->stations[n->get_station()->_id]->eva_nr == "2000058";
      });
  auto cs = get_connections(*node_it, 0);
  ASSERT_EQ(23, cs.size());
  test_events(cs.at(0), exp_time(0, 1630, 180), exp_time(0, 1901, 180));
  // GMT+3 -> GMT+1 (season time)
  test_events(cs.at(8), exp_time(0, 3106, 180), exp_time(0, 3018, 120));
  test_events(cs.at(22), exp_time(0, 5306, 120), exp_time(0, 5716, 120));
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
      begin(sched_->route_index_to_first_route_node),
      end(sched_->route_index_to_first_route_node), [&](node const* n) {
        return sched_->stations[n->get_station()->_id]->eva_nr == "8000080";
      });
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));

  auto cs = get_connections(*node_it, 0);
  ASSERT_EQ(38, cs.size());

  test_events(cs.at(0), exp_time(0, 53, 60), exp_time(0, 55, 60));
  // +1 -> +2
  test_events(cs.at(20), exp_time(0, 158, 60), exp_time(0, 300, 120));
  test_events(cs.at(37), exp_time(0, 344, 120), exp_time(0, 347, 120));
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
      begin(sched_->route_index_to_first_route_node),
      end(sched_->route_index_to_first_route_node), [&](node const* n) {
        return sched_->stations[n->get_station()->_id]->eva_nr == "8000263";
      });
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));

  auto cs = get_connections(*node_it, 0);
  ASSERT_EQ(10, cs.size());

  test_events(cs.at(0), exp_time(0, 108, 60), exp_time(0, 111, 60));
  // +1 -> +2
  test_events(cs.at(9), exp_time(0, 154, 60), exp_time(0, 204, 120 - 60));
}

}  // loader
}  // motis
