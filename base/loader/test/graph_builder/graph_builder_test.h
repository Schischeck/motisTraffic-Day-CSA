#include <ctime>
#include <string>

#include "gtest/gtest.h"

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace loader {

class loader_graph_builder_test : public ::testing::Test {
protected:
  loader_graph_builder_test(std::string schedule_name,
                            std::time_t schedule_begin,
                            std::time_t schedule_end);

  virtual void SetUp();

  static edge const* get_route_edge(node const* route_node);

  static std::vector<
      std::tuple<light_connection const*, node const*, node const*>>
  get_connections(node const* first_route_node, time departure_time);

  schedule_ptr sched_;
  std::string schedule_name_;
  std::time_t schedule_begin_, schedule_end_;
};

}  // loader
}  // motis
