#include "gtest/gtest.h"

#include "motis/module/container.h"

using namespace motis;
using namespace motis::module;

struct my_container {
  my_container(std::string test) : test(std::move(test)) {}
  std::string test;
};

MOTIS_REGISTER_CONTAINER(my_container);

TEST(module_container, register_get_test) {
  snapshot s;
  register_container(s, new my_container("yeah"));
  auto c = get_container<my_container>(s);
  EXPECT_EQ("yeah", c->test);
}
