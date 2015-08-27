#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"
#include "motis/core/schedule/waiting_time_rules.h"

auto schedule = motis::load_schedule("../schedule/test");

TEST_CASE("matrix", "[wzr]") {
  auto const& waiting_time_rules = schedule->waiting_time_rules_;

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

TEST_CASE("family to category assignment", "[wzr]") {
  auto const& waiting_time_rules = schedule->waiting_time_rules_;

  auto it = std::find(begin(schedule->category_names),
                      end(schedule->category_names), "IC");
  ASSERT_TRUE(it != end(schedule->category_names));

  int family = std::distance(begin(schedule->category_names), it);
  ASSERT_TRUE(waiting_time_rules.waiting_time_category(family) == 1);
}

TEST_CASE("train class waits for other trains", "[wzr]") {
  auto const& waiting_time_rules = schedule->waiting_time_rules_;

  ASSERT_TRUE(waiting_time_rules.waits_for_other_trains(1));
  ASSERT_TRUE(!waiting_time_rules.waits_for_other_trains(3));
}

TEST_CASE("other trains wait for train class", "[wzr]") {
  auto const& waiting_time_rules = schedule->waiting_time_rules_;

  ASSERT_TRUE(waiting_time_rules.other_trains_wait_for(1));
  ASSERT_TRUE(!waiting_time_rules.other_trains_wait_for(3));
}
