#include <ctime>
#include <string>

#include "gtest/gtest.h"

#include "motis/core/schedule/schedule.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {

constexpr auto kDefaultTimezoneOffset = 60;

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

  std::time_t unix_time(int hhmm, int day_idx = 0,
                        int timezone_offset = kDefaultTimezoneOffset) {
    return motis_to_unixtime(sched_->schedule_begin_,
                             motis_time(hhmm, day_idx, timezone_offset));
  }

  motis::time motis_time(int hhmm, int day_idx = 0,
                         int timezone_offset = kDefaultTimezoneOffset) {
    return SCHEDULE_OFFSET_MINUTES + day_idx * MINUTES_A_DAY +
           hhmm_to_min(hhmm) - timezone_offset;
  }

  schedule_ptr sched_;
  std::string schedule_name_;
  std::time_t schedule_begin_, schedule_end_;
};

}  // loader
}  // motis
