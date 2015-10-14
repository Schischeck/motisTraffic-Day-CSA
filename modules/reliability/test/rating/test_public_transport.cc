#include "catch/catch.hpp"

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
#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "include/interchange_data_for_tests.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::rating::public_transport;

namespace schedule2 {
struct station {
  std::string name;
  std::string eva;
};
station const ERLANGEN = {"Erlangen", "0953067"};
station const FRANKFURT = {"Frankfurt", "5744986"};
station const KASSEL = {"Kassel", "6380201"};
station const STUTTGART = {"Stuttgart", "7309882"};
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const ICE_E_K = 7;  // 12:45 --> 14:15
}
namespace schedule3 {
struct station {
  std::string name;
  std::string eva;
};
station const FRANKFURT = {"Frankfurt", "1111111"};
station const MESSE = {"Frankfurt Messe", "2222222"};
station const LANGEN = {"Langen", "3333333"};
station const WEST = {"Frankfurt West", "4444444"};
short const ICE_L_H = 1;  // 10:00 --> 10:10
short const S_M_W = 2;  // 10:20 --> 10:25
}
namespace schedule5 {
struct station {
  std::string name;
  std::string eva;
};
station const DARMSTADT = {"Darmstadt", "1111111"};
station const FRANKFURT = {"Frankfurt", "2222222"};
station const GIESSEN = {"Giessen", "3333333"};
station const MARBURG = {"Marburg", "4444444"};
station const BENSHEIM = {"Bensheim", "5555555"};
station const MANNHEIM = {"Mannheim", "6666666"};
short const RE_M_B_D = 3;  // 07:00 --> 07:30, 07:31 --> 07:55
short const RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
short const RE_G_M = 2;  // 09:10 --> 09:40
}

namespace test_public_transport {
auto schedule2 =
    loader::load_schedule("../modules/reliability/resources/schedule2/",
                          to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29));
auto schedule3 =
    loader::load_schedule("../modules/reliability/resources/schedule3/",
                          to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29));
auto schedule5 = loader::load_schedule(
    "../modules/reliability/resources/schedule5/", to_unix_time(2015, 10, 19),
    to_unix_time(2015, 10, 20));
}  // namespace test_public_transport

/* deliver distributions for connection
 * Stuttgart to Erlangen with ICE_S_E (interchange in Stuttgart) and
 * Erlangen to Kassel with ICE_E_K */
std::vector<probability_distribution> compute_test_distributions1(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions) {
  std::vector<probability_distribution> distributions;
  interchange_data_for_tests const ic_data(
      *test_public_transport::schedule2, schedule2::ICE_S_E, schedule2::ICE_E_K,
      schedule2::STUTTGART.eva, schedule2::ERLANGEN.eva, schedule2::KASSEL.eva,
      11 * 60 + 32, 12 * 60 + 32, 12 * 60 + 45, 14 * 60 + 15);

  // departure ICE_S_E in Stuttgart
  distributions.push_back(precomputed_distributions.get_distribution(
      ic_data.arriving_route_edge_._from->_id, 0,
      distributions_container::departure));
  // arrival ICE_S_E in Erlangen
  distributions.push_back(precomputed_distributions.get_distribution(
      ic_data.arriving_route_edge_._to->_id, 0,
      distributions_container::arrival));

  // departure ICE_E_K in Erlangen
  distributions.emplace_back();
  calc_departure_distribution::data_departure_interchange dep_data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, distributions[1],
      *test_public_transport::schedule2, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, distributions.back());

  // arrival ICE_E_K in Kassel
  distributions.emplace_back();
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_._to, ic_data.departing_light_conn_,
      distributions[2], *test_public_transport::schedule2, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(arr_data,
                                                          distributions.back());
  return distributions;
}

TEST_CASE("rate", "[rate_public_transport]") {
  system_tools::setup setup(test_public_transport::schedule2.get());
  auto msg = flatbuffers_tools::to_flatbuffers_message(
      schedule2::STUTTGART.name, schedule2::STUTTGART.eva,
      schedule2::KASSEL.name, schedule2::KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35),
      std::make_tuple(28, 9, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);
    auto const elements = rating::connection_to_graph_data::get_elements(
        *test_public_transport::schedule2, *response->connections()->begin());
    REQUIRE(elements.size() == 2);

    distributions_container::precomputed_distributions_container
        precomputed_distributions(test_public_transport::schedule2->node_count);
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    distributions_calculator::precomputation::perform_precomputation(
        *test_public_transport::schedule2, s_t_distributions,
        precomputed_distributions);
    auto test_distributions = compute_test_distributions1(
        precomputed_distributions, s_t_distributions);
    REQUIRE(test_distributions.size() == 4);

    auto distributions = rate(elements, *test_public_transport::schedule2,
                              precomputed_distributions, s_t_distributions);
    REQUIRE(distributions.size() == 4);
    for (unsigned int i = 0; i < distributions.size(); ++i) {
      REQUIRE(distributions[i] == test_distributions[i]);
    }
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

/* deliver distributions for connection
 * Mannheim to Darmstadt with RE_M_B_D (interchange in Darmstadt),
 * Darmstadt to Giessen with RE_D_F_G (interchange in Giessen), and
 * Giessen to Marburg with RE_G_M */
std::vector<probability_distribution> compute_test_distributions2(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions) {
  std::vector<probability_distribution> distributions;

  /* distributions for the first train (RE_M_B_D) */
  node const& node_m =
      *graph_accessor::get_first_route_node(*test_public_transport::schedule5,
                                            schedule5::RE_M_B_D);
  node const& node_b = *graph_accessor::get_departing_route_edge(node_m)->_to;
  node const& node_d1 = *graph_accessor::get_departing_route_edge(node_b)->_to;
  {
    distributions_container::ride_distributions_container ride_distributions;
    distributions_calculator::ride_distribution::
        compute_distributions_for_a_ride(
            0, node_d1, *test_public_transport::schedule5, s_t_distributions,
            precomputed_distributions, ride_distributions);
    // departure RE_M_B_D in Mannheim
    distributions.push_back(ride_distributions.get_distribution(
        node_m._id, 0, distributions_container::departure));
    // arrival RE_M_B_D in Bensheim
    distributions.push_back(ride_distributions.get_distribution(
        node_b._id, 0, distributions_container::arrival));
    // departure RE_M_B_D in Bensheim
    distributions.push_back(ride_distributions.get_distribution(
        node_b._id, 0, distributions_container::departure));
    // arrival RE_M_B_D in Darmstadt
    distributions.push_back(ride_distributions.get_distribution(
        node_d1._id, 0, distributions_container::arrival));
  }

  /* distributions for RE_D_F_G */

  // departure RE_D_F_G in Darmstadt
  auto const& node_d2 =
      *graph_accessor::get_first_route_node(*test_public_transport::schedule5,
                                            schedule5::RE_D_F_G);
  auto const& edge_d_f = *graph_accessor::get_departing_route_edge(node_d2);
  auto const& lc_d_f = edge_d_f._m._route_edge._conns[0];
  auto const& lc_b_d = graph_accessor::get_departing_route_edge(node_b)
                           ->_m._route_edge._conns[0];
  distributions.emplace_back();
  calc_departure_distribution::data_departure_interchange dep_data(
      true, node_d2, lc_d_f, lc_b_d, distributions[3],
      *test_public_transport::schedule5, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, distributions.back());

  // arrival RE_D_F_G in Frankfurt
  auto const& node_f = *edge_d_f._to;
  distributions.emplace_back();
  calc_arrival_distribution::data_arrival arr_data(
      node_f, lc_d_f, distributions[4], *test_public_transport::schedule5,
      s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(arr_data,
                                                          distributions.back());

  // departure RE_D_F_G in Frankfurt
  auto const& edge_f_g = *graph_accessor::get_departing_route_edge(node_f);
  auto const& lc_f_g = edge_f_g._m._route_edge._conns[0];
  distributions.emplace_back();
  calc_departure_distribution::data_departure dep_data_f(
      node_f, lc_f_g, false, *test_public_transport::schedule5,
      distributions_container::single_distribution_container(distributions[5]),
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::compute_departure_distribution(
      dep_data_f, distributions.back());

  // arrival RE_D_F_G in Giessen
  distributions.emplace_back();
  calc_arrival_distribution::data_arrival arr_data_g(
      *edge_f_g._to, lc_f_g, distributions[6],
      *test_public_transport::schedule5, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(arr_data_g,
                                                          distributions.back());

  /* distributions for RE_G_M */

  // departure RE_G_M in Giessen
  auto const& node_g =
      *graph_accessor::get_first_route_node(*test_public_transport::schedule5,
                                            schedule5::RE_G_M);
  auto const& edge_g_m = *graph_accessor::get_departing_route_edge(node_g);
  auto const& lc_g_m = edge_g_m._m._route_edge._conns[0];
  distributions.emplace_back();
  calc_departure_distribution::data_departure_interchange dep_data_g(
      true, node_g, lc_g_m, lc_f_g, distributions[7],
      *test_public_transport::schedule5, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data_g, distributions.back());

  // arrival RE_G_M in Marburg
  distributions.emplace_back();
  calc_arrival_distribution::data_arrival arr_data_m(
      *edge_g_m._to, lc_g_m, distributions[8],
      *test_public_transport::schedule5, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(arr_data_m,
                                                          distributions.back());
  return distributions;
}

TEST_CASE("rate2", "[rate_public_transport]") {
  system_tools::setup setup(test_public_transport::schedule5.get());
  auto msg = flatbuffers_tools::to_flatbuffers_message(
      schedule5::MANNHEIM.name, schedule5::MANNHEIM.eva,
      schedule5::MARBURG.name, schedule5::MARBURG.eva, (motis::time)(7 * 60),
      (motis::time)(7 * 60 + 1), std::make_tuple(19, 10, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);
    auto const elements = rating::connection_to_graph_data::get_elements(
        *test_public_transport::schedule5, *response->connections()->begin());
    REQUIRE(elements.size() == 3);

    distributions_container::precomputed_distributions_container
        precomputed_distributions(test_public_transport::schedule5->node_count);
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    distributions_calculator::precomputation::perform_precomputation(
        *test_public_transport::schedule5, s_t_distributions,
        precomputed_distributions);
    auto test_distributions = compute_test_distributions2(
        precomputed_distributions, s_t_distributions);
    REQUIRE(test_distributions.size() == 10);

    auto distributions = rate(elements, *test_public_transport::schedule5,
                              precomputed_distributions, s_t_distributions);
    REQUIRE(distributions.size() == 10);
    for (unsigned int i = 0; i < distributions.size(); ++i) {
      REQUIRE(distributions[i] == test_distributions[i]);
    }

    probability_distribution test_distribution;
    test_distribution.init({0.059488, 0.490776, 0.178464, 0.014872}, -1);
    REQUIRE(distributions.back() == test_distribution);
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

/* deliver distributions for connection
 * Langen to Frankfurt Hbf with ICE_L_H (interchange in Frankfurt Hbf),
 * Frankfurt Hbf to Frankfurt Messe via walking, and
 * Frankfurt Messe to Frankfurt West with S_M_W */
std::vector<probability_distribution> compute_test_distributions_foot(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions) {
  std::vector<probability_distribution> distributions;
  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      *test_public_transport::schedule3, schedule3::ICE_L_H, schedule3::S_M_W,
      schedule3::LANGEN.eva, schedule3::FRANKFURT.eva, schedule3::MESSE.eva,
      schedule3::WEST.eva, 10 * 60, 10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);

  // departure ICE_L_H in Langen
  distributions.push_back(precomputed_distributions.get_distribution(
      ic_data.arriving_route_edge_._from->_id, 0,
      distributions_container::departure));
  // arrival ICE_L_H in Frankfurt Hbf
  distributions.push_back(precomputed_distributions.get_distribution(
      ic_data.arriving_route_edge_._to->_id, 0,
      distributions_container::arrival));

  // departure S_M_W in Frankfurt Messe
  distributions.emplace_back();
  calc_departure_distribution::data_departure_interchange_walk dep_data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_._to->_station_node,
      ic_data.departing_light_conn_, ic_data.arriving_light_conn_,
      distributions[1], *test_public_transport::schedule3,
      precomputed_distributions, precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, distributions.back());

  // arrival S_M_W in Frankfurt West
  distributions.emplace_back();
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_._to, ic_data.departing_light_conn_,
      distributions[2], *test_public_transport::schedule3, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(arr_data,
                                                          distributions.back());
  return distributions;
}

TEST_CASE("rate_foot", "[rate_public_transport]") {
  system_tools::setup setup(test_public_transport::schedule3.get());
  auto msg = flatbuffers_tools::to_flatbuffers_message(
      schedule3::LANGEN.name, schedule3::LANGEN.eva, schedule3::WEST.name,
      schedule3::WEST.eva, (motis::time)(10 * 60), (motis::time)(10 * 60 + 1),
      std::make_tuple(28, 9, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);
    auto const elements = rating::connection_to_graph_data::get_elements(
        *test_public_transport::schedule3, *response->connections()->begin());
    REQUIRE(elements.size() == 2);

    distributions_container::precomputed_distributions_container
        precomputed_distributions(test_public_transport::schedule3->node_count);
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    distributions_calculator::precomputation::perform_precomputation(
        *test_public_transport::schedule3, s_t_distributions,
        precomputed_distributions);
    auto test_distributions = compute_test_distributions_foot(
        precomputed_distributions, s_t_distributions);
    REQUIRE(test_distributions.size() == 4);

    auto distributions = rate(elements, *test_public_transport::schedule3,
                              precomputed_distributions, s_t_distributions);
    REQUIRE(distributions.size() == 4);
    for (unsigned int i = 0; i < distributions.size(); ++i) {
      REQUIRE(distributions[i] == test_distributions[i]);
    }

    probability_distribution test_distribution;
    test_distribution.init({0.0592, 0.4932, 0.216, 0.0196}, -1);
    REQUIRE(distributions.back() == test_distribution);
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
