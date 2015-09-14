#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_departure_interchange.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;
using namespace motis::reliability::calc_departure_distribution::interchange;
using namespace motis::reliability::calc_departure_distribution::interchange::
    detail;

TEST_CASE("ic_feeder_arrived", "[calc_departure_distribution_interchange]") {
  probability_distribution pd;
  pd.init({.1, .1, .1, .1, .1, .1, .1, .1}, 0);  // sum = 0.8

  data_departure_interchange::interchange_feeder_info info;
  info.scheduled_arrival_time_ = 0;
  info.transfer_time_ = 4;
  info.waiting_time_ = 3;
  info.latest_feasible_arrival_ = 5;
  info.arrival_distribution_ = &pd;

  for (unsigned int i = 0; i < 8; i++) {
    probability const prob = ic_feeder_arrived(info, (motis::time)(4 + i));
    REQUIRE(equal(prob, (i + 1) * 0.1));
  }
}

TEST_CASE("ic_feeder_arrives_at_time",
          "[calc_departure_distribution_interchange]") {
  probability_distribution pd;
  pd.init({.1, .1, .1, .1, .1, .1, .1, .1}, 0);  // sum = 0.8

  data_departure_interchange::interchange_feeder_info info;
  info.scheduled_arrival_time_ = 0;
  info.transfer_time_ = 4;
  info.waiting_time_ = 3;
  info.latest_feasible_arrival_ = 5;
  info.arrival_distribution_ = &pd;

  for (unsigned int i = 0; i < 8; i++) {
    probability const prob =
        ic_feeder_arrives_at_time(info, (motis::time)(4 + i));
    REQUIRE(equal(prob, 0.1));
  }
}

// first route node without feeders
TEST_CASE("compute_departure_distribution_ic1",
          "[calc_departure_distribution_interchange]") {}
