#include "gtest/gtest.h"

#include <numeric>

#include "motis/core/common/date_util.h"

#include "./graph_builder_test.h"

namespace motis {
namespace loader {

class service_rules_graph_builder_test_real : public loader_graph_builder_test {
public:
  service_rules_graph_builder_test_real()
      : loader_graph_builder_test("cnl-schedule", to_unix_time(2015, 11, 5),
                                  to_unix_time(2015, 11, 7)) {}
};

TEST_F(service_rules_graph_builder_test_real, routes) {}

class service_rules_graph_builder_test_virt : public loader_graph_builder_test {
public:
  service_rules_graph_builder_test_virt()
      : loader_graph_builder_test("mss-ts", to_unix_time(2015, 3, 29),
                                  to_unix_time(2015, 3, 31)) {}
};

TEST_F(service_rules_graph_builder_test_virt, routes) {}

}  // loader
}  // motis
