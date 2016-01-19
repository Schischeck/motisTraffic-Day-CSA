#include "gtest/gtest.h"

#include <numeric>

#include "motis/core/common/date_util.h"

#include "./graph_builder_test.h"

namespace motis {
namespace loader {

std::set<int> get_service_numbers(connection_info const* con) {
  std::set<int> service_numbers;
  while (con != nullptr) {
    service_numbers.insert(con->train_nr);
    con = con->merged_with;
  }
  return service_numbers;
}

class service_rule_graph_builder_test : public loader_graph_builder_test {
public:
  service_rule_graph_builder_test(std::string schedule_name,
                                  std::time_t schedule_begin,
                                  std::time_t schedule_end)
      : loader_graph_builder_test(schedule_name, schedule_begin, schedule_end) {
  }

  std::pair<bool, std::vector<edge const*>> path_exists(std::string const& from,
                                                        std::string const& to) {
    return path_exists(
        sched_->station_nodes[sched_->eva_to_station.at(from)->index].get(),
        sched_->station_nodes[sched_->eva_to_station.at(to)->index].get());
  }

  std::pair<bool, std::vector<edge const*>> path_exists(
      node const* from, station_node const* to,
      std::vector<edge const*> path = std::vector<edge const*>()) {
    if (from->get_station() == to) {
      return std::make_pair(true, path);
    }

    auto nodes = from->is_station_node()
                     ? from->get_station()->get_route_nodes()
                     : std::vector<node const*>({from});
    for (auto const& n : nodes) {
      for (auto const& e : n->_edges) {
        if (e.empty() && e.type() != edge::THROUGH_EDGE) {
          continue;
        }

        auto next_path = path;
        next_path.push_back(&e);
        auto r = path_exists(e.get_destination(), to, std::move(next_path));
        if (r.first) {
          return r;
        }
      }
    }

    return std::make_pair(false, std::vector<edge const*>());
  }
};

class service_rules_graph_builder_test_virt
    : public service_rule_graph_builder_test {
public:
  service_rules_graph_builder_test_virt()
      : service_rule_graph_builder_test("mss-ts", to_unix_time(2015, 3, 29),
                                        to_unix_time(2015, 3, 31)) {}
};

TEST_F(service_rules_graph_builder_test_virt, simple_path_exists) {
  EXPECT_TRUE(path_exists("0000001", "0000003").first);
  EXPECT_FALSE(path_exists("0000003", "0000001").first);
  EXPECT_TRUE(path_exists("0000001", "0000007").first);
}

TEST_F(service_rules_graph_builder_test_virt, through_path_exists) {
  auto path = path_exists("0000005", "0000009");
  EXPECT_TRUE(path.first);
  EXPECT_EQ(path.second[1]->type(), edge::THROUGH_EDGE);
}

TEST_F(service_rules_graph_builder_test_virt, merge_split_path_exists) {
  EXPECT_TRUE(path_exists("0000002", "0000009").first);
  EXPECT_TRUE(path_exists("0000001", "0000011").first);
}

TEST_F(service_rules_graph_builder_test_virt, service_numbers_1) {
  auto path = path_exists("0000003", "0000004");
  ASSERT_TRUE(path.first);

  auto const& e = path.second[0];

  ASSERT_FALSE(e->empty());
  auto train_nrs =
      get_service_numbers(e->_m._route_edge._conns[0]._full_con->con_info);
  EXPECT_TRUE(train_nrs.find(1) != end(train_nrs));
  EXPECT_TRUE(train_nrs.find(2) != end(train_nrs));
  EXPECT_TRUE(train_nrs.find(3) != end(train_nrs));
}

TEST_F(service_rules_graph_builder_test_virt, service_numbers_2) {
  auto path = path_exists("0000004", "0000005");
  ASSERT_TRUE(path.first);

  auto const& e = path.second[0];

  ASSERT_FALSE(e->empty());
  auto train_nrs =
      get_service_numbers(e->_m._route_edge._conns[0]._full_con->con_info);
  EXPECT_TRUE(train_nrs.find(1) != end(train_nrs));
  EXPECT_TRUE(train_nrs.find(2) != end(train_nrs));
}

TEST_F(service_rules_graph_builder_test_virt, service_numbers_3) {
  auto path = path_exists("0000010", "0000011");
  ASSERT_TRUE(path.first);

  auto const& e = path.second[0];

  ASSERT_FALSE(e->empty());
  auto train_nrs =
      get_service_numbers(e->_m._route_edge._conns[0]._full_con->con_info);

  EXPECT_TRUE(train_nrs.find(2) != end(train_nrs));
  EXPECT_TRUE(train_nrs.find(3) != end(train_nrs));
}

}  // loader
}  // motis
