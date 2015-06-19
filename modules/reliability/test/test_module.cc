#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/reliability/reliability.h"

using namespace json11;
using namespace motis::reliability;

TEST_CASE("Initial distributions are calculated", "[initial]") {
  auto schedule = td::load_schedule("../schedule/test");

  reliability rel;
  rel.schedule_ = schedule.get();

  Json msg = Json::object{{"module", "reliability"},
                          {"type", "get-distribution"},
                          {"query-start", "Frankfurt"}};

  auto reply = rel.on_msg(msg, 0);

  REQUIRE(reply.size() == 1);
}
