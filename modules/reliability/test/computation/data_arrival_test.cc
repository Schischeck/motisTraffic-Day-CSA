#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_time_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/graph_accessor.h"

#include "../include/schedules/schedule1.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {

class reliability_data_arrival : public test_schedule_setup {
public:
  reliability_data_arrival()
      : test_schedule_setup(schedule1::PATH, schedule1::DATE) {}
};

TEST_F(reliability_data_arrival, initialize) {
  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->m_.route_edge_.conns_[0];
  auto const& second_route_node = *first_route_edge->to_;

  data_arrival data(*first_route_edge->from_, *first_route_edge->to_,
                    light_connection, dep_dist, *schedule_, s_t_distributions);

  ASSERT_TRUE(
      schedule_->stations_[second_route_node.station_node_->id_]->eva_nr_ ==
      schedule1::DARMSTADT);
  ASSERT_EQ(test_util::minutes_to_motis_time(5 * 60 + 55),
            light_connection.d_time_);
  ASSERT_TRUE(light_connection.a_time_ ==
              test_util::minutes_to_motis_time(6 * 60 + 5));

  ASSERT_EQ(light_connection.d_time_,
            data.departure_info_.scheduled_departure_time_);
  ASSERT_TRUE(&data.departure_info_.distribution_ == &dep_dist);

  ASSERT_TRUE(data.scheduled_arrival_time_ == light_connection.a_time_);

  ASSERT_TRUE(data.travel_distributions_.size() == 2);
  ASSERT_TRUE(&data.travel_distributions_[0].get() ==
              &s_t_distributions.travel_distribution_);
  ASSERT_TRUE(&data.travel_distributions_[1].get() ==
              &s_t_distributions.travel_distribution_);

  ASSERT_TRUE(data.left_bound_ == -1);
  ASSERT_TRUE(data.right_bound_ == 2);
}

TEST_F(reliability_data_arrival, test_s_t_distributions) {
  struct start_and_travel_test2_distributions : start_and_travel_distributions {
    start_and_travel_test2_distributions() {
      distribution_.init_one_point(0, 1.0);
    }
    std::pair<bool, probability_distribution_cref> get_start_distribution(
        std::string const&) const override {
      return std::make_pair(true, std::cref(distribution_));
    }
    bool get_travel_time_distributions(
        std::string const& family, unsigned int const travel_time,
        unsigned int const to_departure_delay,
        std::vector<probability_distribution_cref>& distributions)
        const override {
      if (family == "ICE" && travel_time == 10) {
        for (unsigned int d = 0; d <= to_departure_delay; d++) {
          distributions.push_back(std::cref(distribution_));
        }
        return true;
      }
      return false;
    }
    probability_distribution distribution_;
  } s_t_distributions;

  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto departure_route_node =
      *graph_accessor::get_first_route_node(*schedule_, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const route_edge =
      graph_accessor::get_departing_route_edge(departure_route_node);
  // get the second light connection
  auto const& light_connection = route_edge->m_.route_edge_.conns_[1];

  data_arrival data(*route_edge->from_, *route_edge->to_, light_connection,
                    dep_dist, *schedule_, s_t_distributions);

  ASSERT_TRUE(data.travel_distributions_.size() == 2);
  ASSERT_TRUE(&data.travel_distributions_[0].get() ==
              &s_t_distributions.distribution_);
  ASSERT_TRUE(&data.travel_distributions_[1].get() ==
              &s_t_distributions.distribution_);
}

}  // namespace calc_arrival_distribution
}  // namespace reliability
}  // namespace motis
