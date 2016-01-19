#include "gtest/gtest.h"

#include <climits>
#include <cinttypes>

#include "motis/core/schedule/time.h"

#include "motis/schedule-format/Schedule_generated.h"
#include "./graph_builder_test.h"

namespace motis {
namespace loader {

class loader_direction_services_graph_builder_test
    : public loader_graph_builder_test {
public:
  loader_direction_services_graph_builder_test()
      : loader_graph_builder_test("direction-services",
                                  to_unix_time(2015, 9, 11),
                                  to_unix_time(2015, 9, 12)) {}
};

class loader_graph_builder_never_meet : public loader_graph_builder_test {
public:
  loader_graph_builder_never_meet()
      : loader_graph_builder_test("never-meet", to_unix_time(2015, 1, 4),
                                  to_unix_time(2015, 1, 10)) {}
};

class loader_graph_builder_duplicates_check : public loader_graph_builder_test {
public:
  loader_graph_builder_duplicates_check()
      : loader_graph_builder_test("duplicates", to_unix_time(2015, 1, 4),
                                  to_unix_time(2015, 1, 10)) {}

  uint32_t get_train_num(char const* first_stop_id) {
    auto it = std::find_if(
        begin(sched_->route_index_to_first_route_node),
        end(sched_->route_index_to_first_route_node), [&](node const* n) {
          return sched_->stations[n->get_station()->_id]->eva_nr ==
                 first_stop_id;
        });
    return get<0>(get_connections(*it, 0).at(0))->_full_con->con_info->train_nr;
  }
};

class service_rules_graph_builder_test_real : public loader_graph_builder_test {
public:
  service_rules_graph_builder_test_real()
      : loader_graph_builder_test("cnl-schedule", to_unix_time(2015, 11, 5),
                                  to_unix_time(2015, 11, 7)) {}
};

class service_rules_graph_builder_test_virt : public loader_graph_builder_test {
public:
  service_rules_graph_builder_test_virt()
      : loader_graph_builder_test("mss-ts", to_unix_time(2015, 3, 29),
                                  to_unix_time(2015, 3, 31)) {}

  bool all_stations_exist(std::vector<std::string> const& station_ids) {
    auto const& stations = sched_.get()->stations;
    auto const& station_nodes = sched_.get()->station_nodes;
    return std::all_of(
        begin(station_ids), end(station_ids), [&](std::string const& id) {
          auto s = std::find_if(begin(station_nodes), end(station_nodes),
                                [&](station_node_ptr const& s) {
                                  return stations.at(s.get()->_id)->name == id;
                                });
          if (s == end(station_nodes)) {
            return false;
          } else {
            return true;
          }
        });
  }

  std::vector<edge const*> get_npath(
      std::vector<std::string> const& expected_path) {
    assert(expected_path.size() > 1);

    auto const& stations = sched_.get()->stations;
    auto const& station_nodes = sched_.get()->station_nodes;

    auto const from = std::find_if(begin(station_nodes), end(station_nodes),
                                   [&](station_node_ptr const& s) {
                                     return stations.at(s.get()->_id)->name ==
                                            expected_path.at(0);
                                   });
    if (from == end(station_nodes)) {
      return {};
    }

    std::vector<edge const*> path;
    node const* prev_route_node = nullptr;

    auto augument_path = [&](node const* rn, unsigned i) -> bool {
      for (auto const& re : rn->_edges) {
        if (re.type() == edge::ROUTE_EDGE &&
            stations.at(re._to->_station_node->_id)->name ==
                expected_path.at(i)) {
          path.push_back(&re);
          prev_route_node = path.back()->_to;
          return true;
        }
      }
      return false;
    };

    // find first edge
    for (auto& rn : from->get()->get_route_nodes()) {
      if (augument_path(rn, 1 /* next station*/)) {
        break;
      } else {
        return {};
      }
    }

    // find remaining edges
    for (unsigned i = 2; i < expected_path.size(); ++i) {
      if (augument_path(prev_route_node, i)) {
        continue;
      } else {
        return {};
      }
    }

    return path;
  }

  std::vector<uint32_t> service_numbers(light_connection const* lc) const {
    std::vector<uint32_t> acc;
    auto const* next = lc->_full_con->con_info;
    while (next) {
      acc.push_back(next->train_nr);
      next = next->merged_with;
    }
    return acc;
  }

  int deg_in(node const* route_node) const {
    return std::accumulate(begin(route_node->_edges), end(route_node->_edges),
                           [](int acc, edge const* e) {
                             return e->type() == edge::ROUTE_EDGE ? ++acc : acc;
                           });
  };

  int deg_out(node const* route_node) const {
    return std::accumulate(begin(route_node->_edges), end(route_node->_edges),
                           [](int acc, edge const& e) {
                             return e.type() == edge::ROUTE_EDGE ? ++acc : acc;
                           });
  };
};

TEST_F(loader_direction_services_graph_builder_test, direction_station) {
  // Get route starting at Euskirchen
  auto node_it = std::find_if(
      begin(sched_->route_index_to_first_route_node),
      end(sched_->route_index_to_first_route_node), [&](node const* n) {
        return sched_->stations[n->get_station()->_id]->eva_nr == "8000100";
      });
  ASSERT_FALSE(node_it == end(sched_->route_index_to_first_route_node));

  auto connections = get_connections(*node_it, 0);
  ASSERT_GE(connections.size(), 16);

  for (int i = 0; i < 12; ++i) {
    auto con_info = std::get<0>(connections[i])->_full_con->con_info;
    ASSERT_FALSE(con_info->dir_ == nullptr);
    ASSERT_STREQ("Kreuzberg(Ahr)", con_info->dir_->c_str());
  }

  for (unsigned i = 12; i < connections.size(); ++i) {
    auto con_info = std::get<0>(connections[i])->_full_con->con_info;
    ASSERT_TRUE(con_info->dir_ == nullptr);
  }
}

TEST_F(loader_direction_services_graph_builder_test, direction_text) {
  // Get route starting at Wissmar Gewerbegebiet
  auto node_it = std::find_if(
      begin(sched_->route_index_to_first_route_node),
      end(sched_->route_index_to_first_route_node), [&](node const* n) {
        return sched_->stations[n->get_station()->_id]->eva_nr == "0114965";
      });
  ASSERT_FALSE(node_it == end(sched_->route_index_to_first_route_node));

  auto connections = get_connections(*node_it, 0);
  ASSERT_GE(connections.size(), 27);

  for (auto const& e : connections) {
    auto con_info = std::get<0>(e)->_full_con->con_info;
    ASSERT_FALSE(con_info->dir_ == nullptr);
    ASSERT_STREQ("Krofdorf-Gleiberg Evangelische Ki", con_info->dir_->c_str());
  }
}

TEST_F(loader_graph_builder_never_meet, routes) {
  ASSERT_EQ(3, sched_.get()->route_index_to_first_route_node.size());

  auto node_it = begin(sched_->route_index_to_first_route_node);
  auto connections = get_connections(*node_it, 0);

  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));
  EXPECT_EQ(2, connections.size());
  EXPECT_EQ(1, get<0>(connections[0])->_full_con->con_info->train_nr);

  connections = get_connections(*node_it, get<0>(connections[0])->d_time + 1);
  EXPECT_EQ(2, connections.size());
  EXPECT_EQ(4, get<0>(connections[0])->_full_con->con_info->train_nr);

  node_it = next(node_it, 1);
  connections = get_connections(*node_it, 0);
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));
  EXPECT_EQ(2, connections.size());
  EXPECT_EQ(2, get<0>(connections[0])->_full_con->con_info->train_nr);

  node_it = next(node_it, 1);
  connections = get_connections(*node_it, 0);
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));
  EXPECT_EQ(3, get<0>(connections[0])->_full_con->con_info->train_nr);
}

TEST_F(loader_graph_builder_duplicates_check, duplicate_count) {
  ASSERT_EQ(5, sched_.get()->route_index_to_first_route_node.size());

  auto train_num_zero_count = 0;
  auto train_num_one_count = 0;
  auto duplicate_count = 0;

  for (auto const train_num :
       {get_train_num("0000002"), get_train_num("0000003"),
        get_train_num("0000004"), get_train_num("0000006"),
        get_train_num("0000005")}) {
    if (UINT_MAX - 3 < train_num) {
      ++duplicate_count;
    } else if (train_num == 0) {
      ++train_num_zero_count;
    } else if (train_num == 1) {
      ++train_num_one_count;
    }
  }
  EXPECT_EQ(1, train_num_zero_count);
  EXPECT_EQ(1, train_num_one_count);
  EXPECT_EQ(3, duplicate_count);
}

}  // loader
}  // motis
