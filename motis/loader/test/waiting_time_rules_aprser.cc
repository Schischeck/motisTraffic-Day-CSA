#include "gtest/gtest.h"

#include "motis/loader/waiting_time_rules_parser.h"

namespace motis {
namespace loader {

TEST(wzr, matrix) {
  auto const& waiting_time_rules = parse_waiting_time_rules({});

  ASSERT_TRUE(waiting_time_rules.waiting_time(1, 1) == 3);
  ASSERT_TRUE(waiting_time_rules.waiting_time(1, 2) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(1, 3) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(1, 4) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(1, 5) == 0);

  ASSERT_TRUE(waiting_time_rules.waiting_time(2, 1) == 10);
  ASSERT_TRUE(waiting_time_rules.waiting_time(2, 2) == 10);
  ASSERT_TRUE(waiting_time_rules.waiting_time(2, 3) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(2, 4) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(2, 5) == 5);

  ASSERT_TRUE(waiting_time_rules.waiting_time(3, 1) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(3, 2) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(3, 3) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(3, 4) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(3, 5) == 0);

  ASSERT_TRUE(waiting_time_rules.waiting_time(4, 1) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(4, 2) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(4, 3) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(4, 4) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(4, 5) == 0);

  ASSERT_TRUE(waiting_time_rules.waiting_time(5, 1) == 5);
  ASSERT_TRUE(waiting_time_rules.waiting_time(5, 2) == 5);
  ASSERT_TRUE(waiting_time_rules.waiting_time(5, 3) == 0);
  ASSERT_TRUE(waiting_time_rules.waiting_time(5, 4) == 5);
  ASSERT_TRUE(waiting_time_rules.waiting_time(5, 5) == 5);
}

TEST(wzr, family_to_category_assignment) {
  auto const& waiting_time_rules = parse_waiting_time_rules({"IC"});
  ASSERT_TRUE(waiting_time_rules.waiting_time_category(0) == 1);
}

TEST(wzr, train_class_waits_for_other_trains) {
  auto const& waiting_time_rules = parse_waiting_time_rules({});

  ASSERT_TRUE(waiting_time_rules.waits_for_other_trains(1));
  ASSERT_TRUE(!waiting_time_rules.waits_for_other_trains(3));
}

TEST(wzr, other_trains_wait_for_train_class) {
  auto const& waiting_time_rules = parse_waiting_time_rules({});

  ASSERT_TRUE(waiting_time_rules.other_trains_wait_for(1));
  ASSERT_TRUE(!waiting_time_rules.other_trains_wait_for(3));
}

}  // loader
}  // motis
