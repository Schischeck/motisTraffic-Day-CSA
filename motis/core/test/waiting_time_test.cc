#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/Loader.h"
#include "motis/core/schedule/WaitingTimeRules.h"

auto schedule = td::loadSchedule("../schedule/test");

TEST_CASE("Matrix", "[wzr]") {

  auto const& waitingTimeRules = schedule->waitingTimeRules;

  REQUIRE(waitingTimeRules.waitingTime(1, 1) == 3);
  REQUIRE(waitingTimeRules.waitingTime(1, 2) == 0);
  REQUIRE(waitingTimeRules.waitingTime(1, 3) == 0);
  REQUIRE(waitingTimeRules.waitingTime(1, 4) == 0);
  REQUIRE(waitingTimeRules.waitingTime(1, 5) == 0);

  REQUIRE(waitingTimeRules.waitingTime(2, 1) == 10);
  REQUIRE(waitingTimeRules.waitingTime(2, 2) == 10);
  REQUIRE(waitingTimeRules.waitingTime(2, 3) == 0);
  REQUIRE(waitingTimeRules.waitingTime(2, 4) == 0);
  REQUIRE(waitingTimeRules.waitingTime(2, 5) == 5);

  REQUIRE(waitingTimeRules.waitingTime(3, 1) == 0);
  REQUIRE(waitingTimeRules.waitingTime(3, 2) == 0);
  REQUIRE(waitingTimeRules.waitingTime(3, 3) == 0);
  REQUIRE(waitingTimeRules.waitingTime(3, 4) == 0);
  REQUIRE(waitingTimeRules.waitingTime(3, 5) == 0);

  REQUIRE(waitingTimeRules.waitingTime(4, 1) == 0);
  REQUIRE(waitingTimeRules.waitingTime(4, 2) == 0);
  REQUIRE(waitingTimeRules.waitingTime(4, 3) == 0);
  REQUIRE(waitingTimeRules.waitingTime(4, 4) == 0);
  REQUIRE(waitingTimeRules.waitingTime(4, 5) == 0);

  REQUIRE(waitingTimeRules.waitingTime(5, 1) == 5);
  REQUIRE(waitingTimeRules.waitingTime(5, 2) == 5);
  REQUIRE(waitingTimeRules.waitingTime(5, 3) == 0);
  REQUIRE(waitingTimeRules.waitingTime(5, 4) == 5);
  REQUIRE(waitingTimeRules.waitingTime(5, 5) == 5);
}

TEST_CASE("Family to Category Assignment", "[wzr]") {
  auto const& waitingTimeRules = schedule->waitingTimeRules;

  auto it = std::find(begin(schedule->categoryNames), end(schedule->categoryNames), "IC");
  REQUIRE(it != end(schedule->categoryNames));

  int family = std::distance(begin(schedule->categoryNames), it);
  REQUIRE(waitingTimeRules.waitingTimeCategory(family) == 1);
}

TEST_CASE("Train class waits for other trains", "[wzr]") {
  auto const& waitingTimeRules = schedule->waitingTimeRules;

  REQUIRE(waitingTimeRules.waitsForOtherTrains(1));
  REQUIRE(!waitingTimeRules.waitsForOtherTrains(3));
}

TEST_CASE("Other trains wait for train class", "[wzr]") {
  auto const& waitingTimeRules = schedule->waitingTimeRules;

  REQUIRE(waitingTimeRules.otherTrainsWaitFor(1));
  REQUIRE(!waitingTimeRules.otherTrainsWaitFor(3));
}
