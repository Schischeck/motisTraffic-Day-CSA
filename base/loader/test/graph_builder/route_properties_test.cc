#include "gtest/gtest.h"

#include <climits>
#include <cinttypes>

#include "motis/core/schedule/time.h"
#include "motis/core/common/date_time_util.h"

#include "./graph_builder_test.h"

namespace motis {
namespace loader {

class loader_graph_builder_never_meet : public loader_graph_builder_test {
public:
  loader_graph_builder_never_meet()
      : loader_graph_builder_test("never-meet", to_unix_time(2015, 1, 4),
                                  to_unix_time(2015, 1, 10)) {}
};

TEST_F(loader_graph_builder_never_meet, routes) {
  ASSERT_EQ(3, sched_.get()->route_index_to_first_route_node.size());

  auto node_it = begin(sched_->route_index_to_first_route_node);
  auto connections = get_connections(*node_it, 0);

  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));
  EXPECT_EQ(2, connections.size());
  EXPECT_EQ(1, std::get<0>(connections[0])->_full_con->con_info->train_nr);

  connections =
      get_connections(*node_it, std::get<0>(connections[0])->d_time + 1);
  EXPECT_EQ(2, connections.size());
  EXPECT_EQ(4, std::get<0>(connections[0])->_full_con->con_info->train_nr);

  node_it = next(node_it, 1);
  connections = get_connections(*node_it, 0);
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));
  EXPECT_EQ(2, connections.size());
  EXPECT_EQ(2, std::get<0>(connections[0])->_full_con->con_info->train_nr);

  node_it = next(node_it, 1);
  connections = get_connections(*node_it, 0);
  ASSERT_TRUE(node_it != end(sched_->route_index_to_first_route_node));
  EXPECT_EQ(3, std::get<0>(connections[0])->_full_con->con_info->train_nr);
}

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
    return std::get<0>(get_connections(*it, 0).at(0))
        ->_full_con->con_info->train_nr;
  }
};

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
