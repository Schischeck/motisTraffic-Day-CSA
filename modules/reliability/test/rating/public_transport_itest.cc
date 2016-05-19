#include "gtest/gtest.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/computation/ride_distributions_calculator.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/schedules/schedule2.h"
#include "../include/schedules/schedule3.h"
#include "../include/schedules/schedule5.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace rating {
namespace public_transport {

class reliability_public_transport2 : public test_motis_setup {
public:
  reliability_public_transport2()
      : test_motis_setup(schedule2::PATH, schedule2::DATE) {}
};
class reliability_public_transport3 : public test_motis_setup {
public:
  reliability_public_transport3()
      : test_motis_setup(schedule3::PATH, schedule3::DATE) {}
};
class reliability_public_transport5 : public test_motis_setup {
public:
  reliability_public_transport5()
      : test_motis_setup(schedule5::PATH, schedule5::DATE) {}
};

/* deliver distributions for connection
 * Stuttgart to Erlangen with ICE_S_E (interchange in Stuttgart) and
 * Erlangen to Kassel with ICE_E_K */
std::vector<rating::rating_element> compute_test_ratings1(
    distributions_container::container const& precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions,
    reliability_public_transport2 const& test_info) {
  std::vector<rating::rating_element> ratings;
  interchange_data_for_tests const ic_data(
      test_info.get_schedule(), schedule2::ICE_S_E, schedule2::ICE_E_K,
      schedule2::STUTTGART.eva_, schedule2::ERLANGEN.eva_,
      schedule2::KASSEL.eva_, 11 * 60 + 32, 12 * 60 + 32, 12 * 60 + 45,
      14 * 60 + 15);

  // departure ICE_S_E in Stuttgart
  ratings.emplace_back(0);
  ratings.back().departure_distribution_ =
      precomputed_distributions.get_distribution(
          distributions_container::to_container_key(
              *ic_data.arriving_route_edge_.from_, ic_data.arriving_light_conn_,
              time_util::departure, test_info.reliability_context_->schedule_));
  // arrival ICE_S_E in Erlangen
  ratings.back().arrival_distribution_ =
      precomputed_distributions.get_distribution(
          distributions_container::to_container_key(
              *ic_data.arriving_route_edge_.to_, ic_data.arriving_light_conn_,
              time_util::arrival, test_info.reliability_context_->schedule_));

  // departure ICE_E_K in Erlangen
  ratings.emplace_back(1);
  calc_departure_distribution::data_departure_interchange dep_data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, ratings[0].arrival_distribution_,
      precomputed_distributions,
      precomputed_distributions.get_node(
          distributions_container::to_container_key(
              ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
              time_util::departure, test_info.get_schedule())),
      context(test_info.get_schedule(), precomputed_distributions,
              s_t_distributions));
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, ratings.back().departure_distribution_);

  // arrival ICE_E_K in Kassel
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_.from_, *ic_data.departing_route_edge_.to_,
      ic_data.departing_light_conn_, ratings.back().departure_distribution_,
      test_info.get_schedule(), s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data, ratings.back().arrival_distribution_);
  return ratings;
}

TEST_F(reliability_public_transport2, rate) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule2::STUTTGART.name_,
                             schedule2::STUTTGART.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 1132),
                             test_util::hhmm_to_unixtime(get_schedule(), 1132))
          .add_destination(schedule2::KASSEL.name_, schedule2::KASSEL.eva_)
          .build_routing_request();
  auto msg = call(req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_EQ(1, journeys.size());
  auto const elements = rating::connection_to_graph_data::get_elements(
      get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);

  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  auto test_ratings = compute_test_ratings1(
      get_reliability_module().precomputed_distributions(), s_t_distributions,
      *this);
  ASSERT_TRUE(test_ratings.size() == 2);

  std::vector<rating::rating_element> ratings;
  rate(ratings, elements, false, *reliability_context_);
  ASSERT_TRUE(ratings.size() == 2);
  for (unsigned int i = 0; i < ratings.size(); ++i) {
    ASSERT_EQ(test_ratings[i].departure_stop_idx_,
              ratings[i].departure_stop_idx_);
    ASSERT_EQ(test_ratings[i].departure_distribution_,
              ratings[i].departure_distribution_);
    ASSERT_EQ(test_ratings[i].arrival_distribution_,
              ratings[i].arrival_distribution_);
  }
}

/* deliver distributions for connection
 * Mannheim to Darmstadt with RE_M_B_D (interchange in Darmstadt),
 * Darmstadt to Giessen with RE_D_F_G (interchange in Giessen), and
 * Giessen to Marburg with RE_G_M */
std::vector<rating::rating_element> compute_test_ratings2(
    context const& c, reliability_public_transport5 const& test_info) {
  std::vector<rating::rating_element> ratings;

  /* distributions for the first train (RE_M_B_D) */
  node const& node_m = *graph_accessor::get_first_route_node(
      test_info.get_schedule(), schedule5::RE_M_B_D);
  node const& node_b = *graph_accessor::get_departing_route_edge(node_m)->to_;
  node const& node_d1 = *graph_accessor::get_departing_route_edge(node_b)->to_;
  {
    distributions_container::container ride_distributions;
    distributions_calculator::ride_distribution::detail::
        compute_distributions_for_a_ride(0, node_d1, c, ride_distributions);
    ratings.emplace_back(0);
    // departure RE_M_B_D in Mannheim
    ratings.back().departure_distribution_ =
        ride_distributions.get_distribution(
            distributions_container::to_container_key(
                node_m, graph_accessor::get_departing_route_edge(node_m)
                            ->m_.route_edge_.conns_.front(),
                time_util::departure,
                test_info.reliability_context_->schedule_));
    // arrival RE_M_B_D in Bensheim
    ratings.back().arrival_distribution_ = ride_distributions.get_distribution(
        distributions_container::to_container_key(
            node_b, graph_accessor::get_departing_route_edge(node_m)
                        ->m_.route_edge_.conns_.front(),
            time_util::arrival, test_info.reliability_context_->schedule_));
    ratings.emplace_back(1);
    // departure RE_M_B_D in Bensheim
    ratings.back().departure_distribution_ =
        ride_distributions.get_distribution(
            distributions_container::to_container_key(
                node_b, graph_accessor::get_departing_route_edge(node_b)
                            ->m_.route_edge_.conns_.front(),
                time_util::departure,
                test_info.reliability_context_->schedule_));
    // arrival RE_M_B_D in Darmstadt
    ratings.back().arrival_distribution_ = ride_distributions.get_distribution(
        distributions_container::to_container_key(
            node_d1, graph_accessor::get_departing_route_edge(node_b)
                         ->m_.route_edge_.conns_.front(),
            time_util::arrival, test_info.reliability_context_->schedule_));
  }

  /* distributions for RE_D_F_G */

  // departure RE_D_F_G in Darmstadt
  auto const& node_d2 = *graph_accessor::get_first_route_node(
      test_info.get_schedule(), schedule5::RE_D_F_G);
  auto const& edge_d_f = *graph_accessor::get_departing_route_edge(node_d2);
  auto const& lc_d_f = edge_d_f.m_.route_edge_.conns_[0];
  auto const& lc_b_d = graph_accessor::get_departing_route_edge(node_b)
                           ->m_.route_edge_.conns_[0];
  ratings.emplace_back(2);
  calc_departure_distribution::data_departure_interchange dep_data(
      true, node_d2, node_d1, lc_d_f, lc_b_d, ratings[1].arrival_distribution_,
      c.precomputed_distributions_,
      c.precomputed_distributions_.get_node(
          distributions_container::to_container_key(
              node_d2, lc_d_f, time_util::departure, c.schedule_)),
      context(test_info.get_schedule(), c.precomputed_distributions_,
              c.s_t_distributions_));
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, ratings.back().departure_distribution_);

  // arrival RE_D_F_G in Frankfurt
  auto const& node_f = *edge_d_f.to_;
  calc_arrival_distribution::data_arrival arr_data(
      *edge_d_f.from_, *edge_d_f.to_, lc_d_f,
      ratings.back().departure_distribution_, test_info.get_schedule(),
      c.s_t_distributions_);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data, ratings.back().arrival_distribution_);

  // departure RE_D_F_G in Frankfurt
  auto const& edge_f_g = *graph_accessor::get_departing_route_edge(node_f);
  auto const& lc_f_g = edge_f_g.m_.route_edge_.conns_[0];
  ratings.emplace_back(3);
  calc_departure_distribution::data_departure dep_data_f(
      node_f, lc_f_g, false,
      distributions_container::single_distribution_container(
          ratings[2].arrival_distribution_),
      c.precomputed_distributions_.get_node(
          distributions_container::to_container_key(
              node_f, lc_f_g, time_util::departure, c.schedule_)),
      context(test_info.get_schedule(), c.precomputed_distributions_,
              c.s_t_distributions_));
  calc_departure_distribution::compute_departure_distribution(
      dep_data_f, ratings.back().departure_distribution_);

  // arrival RE_D_F_G in Giessen
  calc_arrival_distribution::data_arrival arr_data_g(
      *edge_f_g.from_, *edge_f_g.to_, lc_f_g,
      ratings.back().departure_distribution_, test_info.get_schedule(),
      c.s_t_distributions_);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data_g, ratings.back().arrival_distribution_);

  /* distributions for RE_G_M */

  // departure RE_G_M in Giessen
  auto const& node_g = *graph_accessor::get_first_route_node(
      test_info.get_schedule(), schedule5::RE_G_M);
  auto const& edge_g_m = *graph_accessor::get_departing_route_edge(node_g);
  auto const& lc_g_m = edge_g_m.m_.route_edge_.conns_[0];
  ratings.emplace_back(4);
  calc_departure_distribution::data_departure_interchange dep_data_g(
      true, node_g, *edge_f_g.to_, lc_g_m, lc_f_g,
      ratings[3].arrival_distribution_, c.precomputed_distributions_,
      c.precomputed_distributions_.get_node(
          distributions_container::to_container_key(
              node_g, lc_g_m, time_util::departure, c.schedule_)),
      context(test_info.get_schedule(), c.precomputed_distributions_,
              c.s_t_distributions_));
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data_g, ratings.back().departure_distribution_);

  // arrival RE_G_M in Marburg
  calc_arrival_distribution::data_arrival arr_data_m(
      *edge_g_m.from_, *edge_g_m.to_, lc_g_m,
      ratings.back().departure_distribution_, test_info.get_schedule(),
      c.s_t_distributions_);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data_m, ratings.back().arrival_distribution_);
  return ratings;
}

TEST_F(reliability_public_transport5, rate2) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule5::MANNHEIM.name_,
                             schedule5::MANNHEIM.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 700))
          .add_destination(schedule5::MARBURG.name_, schedule5::MARBURG.eva_)
          .build_routing_request();
  auto msg = call(req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_TRUE(journeys.size() == 1);
  auto const elements = rating::connection_to_graph_data::get_elements(
      get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 3);

  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  auto test_ratings = compute_test_ratings2(*reliability_context_, *this);
  ASSERT_TRUE(test_ratings.size() == 5);

  std::vector<rating::rating_element> ratings;
  rate(ratings, elements, false, *reliability_context_);
  ASSERT_TRUE(ratings.size() == 5);
  for (unsigned int i = 0; i < ratings.size(); ++i) {
    ASSERT_TRUE(ratings[i].departure_stop_idx_ ==
                test_ratings[i].departure_stop_idx_);
    ASSERT_TRUE(ratings[i].departure_distribution_ ==
                test_ratings[i].departure_distribution_);
    ASSERT_TRUE(ratings[i].arrival_distribution_ ==
                test_ratings[i].arrival_distribution_);
  }

  probability_distribution test_distribution;
  test_distribution.init({0.059488, 0.490776, 0.178464, 0.014872}, -1);
  ASSERT_TRUE(ratings.back().arrival_distribution_ == test_distribution);
}

/* deliver distributions for connection
 * Langen to Frankfurt Hbf with ICE_L_H (interchange in Frankfurt Hbf),
 * Frankfurt Hbf to Frankfurt Messe via walking, and
 * Frankfurt Messe to Frankfurt West with S_M_W */
std::vector<rating::rating_element> compute_test_ratings_foot(
    distributions_container::container const& precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions,
    reliability_public_transport3 const& test_info) {
  std::vector<rating::rating_element> ratings;
  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      test_info.get_schedule(), schedule3::ICE_L_H, schedule3::S_M_W,
      schedule3::LANGEN.eva_, schedule3::FRANKFURT.eva_, schedule3::MESSE.eva_,
      schedule3::WEST.eva_, 10 * 60, 10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);

  // departure ICE_L_H in Langen
  ratings.emplace_back(0);
  ratings.back().departure_distribution_ =
      precomputed_distributions.get_distribution(
          distributions_container::to_container_key(
              *ic_data.arriving_route_edge_.from_, ic_data.arriving_light_conn_,
              time_util::departure, test_info.reliability_context_->schedule_));
  // arrival ICE_L_H in Frankfurt Hbf
  ratings.back().arrival_distribution_ =
      precomputed_distributions.get_distribution(
          distributions_container::to_container_key(
              *ic_data.arriving_route_edge_.to_, ic_data.arriving_light_conn_,
              time_util::arrival, test_info.reliability_context_->schedule_));

  // departure S_M_W in Frankfurt Messe
  ratings.emplace_back(2);
  calc_departure_distribution::data_departure_interchange_walk dep_data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, ratings[0].arrival_distribution_,
      precomputed_distributions,
      precomputed_distributions.get_node(
          distributions_container::to_container_key(
              ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
              time_util::departure, test_info.get_schedule())),
      context(test_info.get_schedule(), precomputed_distributions,
              s_t_distributions));
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, ratings.back().departure_distribution_);

  // arrival S_M_W in Frankfurt West
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_.from_, *ic_data.departing_route_edge_.to_,
      ic_data.departing_light_conn_, ratings.back().departure_distribution_,
      test_info.get_schedule(), s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data, ratings.back().arrival_distribution_);
  return ratings;
}

TEST_F(reliability_public_transport3, rate_foot) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule3::LANGEN.name_, schedule3::LANGEN.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 1000),
                             test_util::hhmm_to_unixtime(get_schedule(), 1000))
          .add_destination(schedule3::WEST.name_, schedule3::WEST.eva_)
          .build_routing_request();
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, call(req_msg)));

  ASSERT_EQ(1, journeys.size());
  auto const elements = rating::connection_to_graph_data::get_elements(
      get_schedule(), journeys.front());
  ASSERT_EQ(2, elements.size());

  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  auto test_ratings = compute_test_ratings_foot(
      get_reliability_module().precomputed_distributions(), s_t_distributions,
      *this);
  ASSERT_TRUE(test_ratings.size() == 2);

  std::vector<rating::rating_element> ratings;
  rate(ratings, elements, false, *reliability_context_);
  ASSERT_TRUE(ratings.size() == 2);
  for (unsigned int i = 0; i < ratings.size(); ++i) {
    ASSERT_TRUE(ratings[i].departure_stop_idx_ ==
                test_ratings[i].departure_stop_idx_);
    ASSERT_TRUE(ratings[i].departure_distribution_ ==
                test_ratings[i].departure_distribution_);
    ASSERT_TRUE(ratings[i].arrival_distribution_ ==
                test_ratings[i].arrival_distribution_);
  }

  probability_distribution test_distribution;
  test_distribution.init({0.0592, 0.4932, 0.216, 0.0196}, -1);
  ASSERT_TRUE(ratings.back().arrival_distribution_ == test_distribution);
}

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
