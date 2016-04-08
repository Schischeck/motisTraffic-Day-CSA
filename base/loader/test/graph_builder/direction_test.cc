#include "gtest/gtest.h"

#include "motis/core/common/date_time_util.h"

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

}  // loader
}  // motis
