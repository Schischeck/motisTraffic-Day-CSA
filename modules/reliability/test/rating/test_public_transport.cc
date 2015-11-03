#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"
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
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "include/interchange_data_for_tests.h"
#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace rating {
namespace public_transport {

class test_public_transport2 : public test_schedule_setup {
public:
  test_public_transport2()
      : test_schedule_setup("modules/reliability/resources/schedule2/",
                            to_unix_time(2015, 9, 28),
                            to_unix_time(2015, 9, 29)) {}
  schedule_station const ERLANGEN = {"Erlangen", "0953067"};
  schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
  schedule_station const KASSEL = {"Kassel", "6380201"};
  schedule_station const STUTTGART = {"Stuttgart", "7309882"};
  short const ICE_S_E = 5;  // 11:32 --> 12:32
  short const ICE_E_K = 7;  // 12:45 --> 14:15
};
class test_public_transport3 : public test_schedule_setup {
public:
  test_public_transport3()
      : test_schedule_setup("modules/reliability/resources/schedule3/",
                            to_unix_time(2015, 9, 28),
                            to_unix_time(2015, 9, 29)) {}
  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const MESSE = {"Frankfurt Messe", "2222222"};
  schedule_station const LANGEN = {"Langen", "3333333"};
  schedule_station const WEST = {"Frankfurt West", "4444444"};
  short const ICE_L_H = 1;  // 10:00 --> 10:10
  short const S_M_W = 2;  // 10:20 --> 10:25
};
class test_public_transport5 : public test_schedule_setup {
public:
  test_public_transport5()
      : test_schedule_setup("modules/reliability/resources/schedule5/",
                            to_unix_time(2015, 10, 19),
                            to_unix_time(2015, 10, 20)) {}
  schedule_station const DARMSTADT = {"Darmstadt", "1111111"};
  schedule_station const FRANKFURT = {"Frankfurt", "2222222"};
  schedule_station const GIESSEN = {"Giessen", "3333333"};
  schedule_station const MARBURG = {"Marburg", "4444444"};
  schedule_station const BENSHEIM = {"Bensheim", "5555555"};
  schedule_station const MANNHEIM = {"Mannheim", "6666666"};
  short const RE_M_B_D = 3;  // 07:00 --> 07:30, 07:31 --> 07:55
  short const RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
  short const RE_G_M = 2;  // 09:10 --> 09:40
};

/* deliver distributions for connection
 * Stuttgart to Erlangen with ICE_S_E (interchange in Stuttgart) and
 * Erlangen to Kassel with ICE_E_K */
std::vector<rating::rating_element> compute_test_ratings1(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions,
    test_public_transport2 const& test_info) {
  std::vector<rating::rating_element> ratings;
  interchange_data_for_tests const ic_data(
      *test_info.schedule_, test_info.ICE_S_E, test_info.ICE_E_K,
      test_info.STUTTGART.eva, test_info.ERLANGEN.eva, test_info.KASSEL.eva,
      11 * 60 + 32, 12 * 60 + 32, 12 * 60 + 45, 14 * 60 + 15);

  // departure ICE_S_E in Stuttgart
  ratings.emplace_back(1);
  ratings.back().departure_distribution_ =
      precomputed_distributions.get_distribution(
          ic_data.arriving_route_edge_._from->_id, 0,
          distributions_container::departure);
  // arrival ICE_S_E in Erlangen
  ratings.back().arrival_distribution_ =
      precomputed_distributions.get_distribution(
          ic_data.arriving_route_edge_._to->_id, 0,
          distributions_container::arrival);

  // departure ICE_E_K in Erlangen
  ratings.emplace_back(2);
  calc_departure_distribution::data_departure_interchange dep_data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, ratings[0].arrival_distribution_,
      *test_info.schedule_, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, ratings.back().departure_distribution_);

  // arrival ICE_E_K in Kassel
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_._to, ic_data.departing_light_conn_,
      ratings.back().departure_distribution_, *test_info.schedule_,
      s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data, ratings.back().arrival_distribution_);
  return ratings;
}

TEST_F(test_public_transport2, rate) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_routing_request(
      STUTTGART.name, STUTTGART.eva, KASSEL.name, KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35),
      std::make_tuple(28, 9, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    ASSERT_TRUE(response->connections()->size() == 1);
    auto const elements =
        rating::connection_to_graph_data::get_elements(
            *schedule_, *response->connections()->begin()).second;
    ASSERT_TRUE(elements.size() == 2);

    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    auto test_ratings = compute_test_ratings1(
        setup.reliability_module().precomputed_distributions(),
        s_t_distributions, *this);
    ASSERT_TRUE(test_ratings.size() == 2);

    std::vector<rating::rating_element> ratings;
    rate(ratings, elements, *schedule_,
         setup.reliability_module().precomputed_distributions(),
         s_t_distributions);
    ASSERT_TRUE(ratings.size() == 2);
    for (unsigned int i = 0; i < ratings.size(); ++i) {
      ASSERT_TRUE(ratings[i].departure_stop_idx_ ==
                  test_ratings[i].departure_stop_idx_);
      ASSERT_TRUE(ratings[i].departure_distribution_ ==
                  test_ratings[i].departure_distribution_);
      ASSERT_TRUE(ratings[i].arrival_distribution_ ==
                  test_ratings[i].arrival_distribution_);
    }
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

/* deliver distributions for connection
 * Mannheim to Darmstadt with RE_M_B_D (interchange in Darmstadt),
 * Darmstadt to Giessen with RE_D_F_G (interchange in Giessen), and
 * Giessen to Marburg with RE_G_M */
std::vector<rating::rating_element> compute_test_ratings2(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions,
    test_public_transport5 const& test_info) {
  std::vector<rating::rating_element> ratings;

  /* distributions for the first train (RE_M_B_D) */
  node const& node_m = *graph_accessor::get_first_route_node(
                           *test_info.schedule_, test_info.RE_M_B_D);
  node const& node_b = *graph_accessor::get_departing_route_edge(node_m)->_to;
  node const& node_d1 = *graph_accessor::get_departing_route_edge(node_b)->_to;
  {
    distributions_container::ride_distributions_container ride_distributions;
    distributions_calculator::ride_distribution::detail::
        compute_distributions_for_a_ride(
            0, node_d1, *test_info.schedule_, s_t_distributions,
            precomputed_distributions, ride_distributions);
    ratings.emplace_back(1);
    // departure RE_M_B_D in Mannheim
    ratings.back().departure_distribution_ =
        ride_distributions.get_distribution(node_m._id, 0,
                                            distributions_container::departure);
    // arrival RE_M_B_D in Bensheim
    ratings.back().arrival_distribution_ = ride_distributions.get_distribution(
        node_b._id, 0, distributions_container::arrival);
    ratings.emplace_back(2);
    // departure RE_M_B_D in Bensheim
    ratings.back().departure_distribution_ =
        ride_distributions.get_distribution(node_b._id, 0,
                                            distributions_container::departure);
    // arrival RE_M_B_D in Darmstadt
    ratings.back().arrival_distribution_ = ride_distributions.get_distribution(
        node_d1._id, 0, distributions_container::arrival);
  }

  /* distributions for RE_D_F_G */

  // departure RE_D_F_G in Darmstadt
  auto const& node_d2 = *graph_accessor::get_first_route_node(
                            *test_info.schedule_, test_info.RE_D_F_G);
  auto const& edge_d_f = *graph_accessor::get_departing_route_edge(node_d2);
  auto const& lc_d_f = edge_d_f._m._route_edge._conns[0];
  auto const& lc_b_d = graph_accessor::get_departing_route_edge(node_b)
                           ->_m._route_edge._conns[0];
  ratings.emplace_back(3);
  calc_departure_distribution::data_departure_interchange dep_data(
      true, node_d2, lc_d_f, lc_b_d, ratings[1].arrival_distribution_,
      *test_info.schedule_, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, ratings.back().departure_distribution_);

  // arrival RE_D_F_G in Frankfurt
  auto const& node_f = *edge_d_f._to;
  calc_arrival_distribution::data_arrival arr_data(
      node_f, lc_d_f, ratings.back().departure_distribution_,
      *test_info.schedule_, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data, ratings.back().arrival_distribution_);

  // departure RE_D_F_G in Frankfurt
  auto const& edge_f_g = *graph_accessor::get_departing_route_edge(node_f);
  auto const& lc_f_g = edge_f_g._m._route_edge._conns[0];
  ratings.emplace_back(4);
  calc_departure_distribution::data_departure dep_data_f(
      node_f, lc_f_g, false, *test_info.schedule_,
      distributions_container::single_distribution_container(
          ratings[2].arrival_distribution_),
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::compute_departure_distribution(
      dep_data_f, ratings.back().departure_distribution_);

  // arrival RE_D_F_G in Giessen
  calc_arrival_distribution::data_arrival arr_data_g(
      *edge_f_g._to, lc_f_g, ratings.back().departure_distribution_,
      *test_info.schedule_, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data_g, ratings.back().arrival_distribution_);

  /* distributions for RE_G_M */

  // departure RE_G_M in Giessen
  auto const& node_g = *graph_accessor::get_first_route_node(
                           *test_info.schedule_, test_info.RE_G_M);
  auto const& edge_g_m = *graph_accessor::get_departing_route_edge(node_g);
  auto const& lc_g_m = edge_g_m._m._route_edge._conns[0];
  ratings.emplace_back(5);
  calc_departure_distribution::data_departure_interchange dep_data_g(
      true, node_g, lc_g_m, lc_f_g, ratings[3].arrival_distribution_,
      *test_info.schedule_, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data_g, ratings.back().departure_distribution_);

  // arrival RE_G_M in Marburg
  calc_arrival_distribution::data_arrival arr_data_m(
      *edge_g_m._to, lc_g_m, ratings.back().departure_distribution_,
      *test_info.schedule_, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data_m, ratings.back().arrival_distribution_);
  return ratings;
}

TEST_F(test_public_transport5, rate2) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_routing_request(
      MANNHEIM.name, MANNHEIM.eva, MARBURG.name, MARBURG.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    ASSERT_TRUE(response->connections()->size() == 1);
    auto const elements =
        rating::connection_to_graph_data::get_elements(
            *schedule_, *response->connections()->begin()).second;
    ASSERT_TRUE(elements.size() == 3);

    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    auto test_ratings = compute_test_ratings2(
        setup.reliability_module().precomputed_distributions(),
        s_t_distributions, *this);
    ASSERT_TRUE(test_ratings.size() == 5);

    std::vector<rating::rating_element> ratings;
    rate(ratings, elements, *schedule_,
         setup.reliability_module().precomputed_distributions(),
         s_t_distributions);
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
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

/* deliver distributions for connection
 * Langen to Frankfurt Hbf with ICE_L_H (interchange in Frankfurt Hbf),
 * Frankfurt Hbf to Frankfurt Messe via walking, and
 * Frankfurt Messe to Frankfurt West with S_M_W */
std::vector<rating::rating_element> compute_test_ratings_foot(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions,
    test_public_transport3 const& test_info) {
  std::vector<rating::rating_element> ratings;
  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      *test_info.schedule_, test_info.ICE_L_H, test_info.S_M_W,
      test_info.LANGEN.eva, test_info.FRANKFURT.eva, test_info.MESSE.eva,
      test_info.WEST.eva, 10 * 60, 10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);

  // departure ICE_L_H in Langen
  ratings.emplace_back(1);
  ratings.back().departure_distribution_ =
      precomputed_distributions.get_distribution(
          ic_data.arriving_route_edge_._from->_id, 0,
          distributions_container::departure);
  // arrival ICE_L_H in Frankfurt Hbf
  ratings.back().arrival_distribution_ =
      precomputed_distributions.get_distribution(
          ic_data.arriving_route_edge_._to->_id, 0,
          distributions_container::arrival);

  // departure S_M_W in Frankfurt Messe
  ratings.emplace_back(3);
  calc_departure_distribution::data_departure_interchange_walk dep_data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_._to->_station_node,
      ic_data.departing_light_conn_, ic_data.arriving_light_conn_,
      ratings[0].arrival_distribution_, *test_info.schedule_,
      precomputed_distributions, precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, ratings.back().departure_distribution_);

  // arrival S_M_W in Frankfurt West
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_._to, ic_data.departing_light_conn_,
      ratings.back().departure_distribution_, *test_info.schedule_,
      s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(
      arr_data, ratings.back().arrival_distribution_);
  return ratings;
}

TEST_F(test_public_transport3, rate_foot) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_routing_request(
      LANGEN.name, LANGEN.eva, WEST.name, WEST.eva, (motis::time)(10 * 60),
      (motis::time)(10 * 60 + 1), std::make_tuple(28, 9, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    ASSERT_TRUE(response->connections()->size() == 1);
    auto const elements =
        rating::connection_to_graph_data::get_elements(
            *schedule_, *response->connections()->begin()).second;
    ASSERT_TRUE(elements.size() == 2);

    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    auto test_ratings = compute_test_ratings_foot(
        setup.reliability_module().precomputed_distributions(),
        s_t_distributions, *this);
    ASSERT_TRUE(test_ratings.size() == 2);

    std::vector<rating::rating_element> ratings;
    rate(ratings, elements, *schedule_,
         setup.reliability_module().precomputed_distributions(),
         s_t_distributions);
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
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
