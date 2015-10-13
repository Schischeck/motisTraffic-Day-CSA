#include "catch/catch.hpp"

#include "motis/core/common/date_util.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
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

namespace test_public_transport {
auto schedule =
    loader::load_schedule("../modules/reliability/resources/schedule2/",
                          to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29));
}  // namespace test_public_transport

/* deliver distributions for connection
 * Stuttgart to Erlangen with ICE_S_E (interchange in Stuttgart) and
 * Erlangen to Kassel with ICE_E_K */
std::vector<probability_distribution> compute_test_distributions(
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions) {
  using namespace test_public_transport;
  start_and_travel_test_distributions s_t_distributions({.6, .4});
  std::vector<probability_distribution> distributions;
  interchange_data_for_tests const ic_data(
      *test_public_transport::schedule, schedule2::ICE_S_E, schedule2::ICE_E_K,
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
  distributions.push_back(probability_distribution());
  calc_departure_distribution::data_departure_interchange dep_data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, distributions[1],
      *test_public_transport::schedule, precomputed_distributions,
      precomputed_distributions, s_t_distributions);
  calc_departure_distribution::interchange::compute_departure_distribution(
      dep_data, distributions[2]);

  // arrival ICE_E_K in Kassel
  distributions.push_back(probability_distribution());
  calc_arrival_distribution::data_arrival arr_data(
      *ic_data.departing_route_edge_._to, ic_data.departing_light_conn_,
      distributions[2], *test_public_transport::schedule, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(arr_data,
                                                          distributions[3]);
  return distributions;
}

TEST_CASE("rate", "[rate_public_transport]") {
  system_tools::setup setup(test_public_transport::schedule.get());
  auto msg = flatbuffers_tools::to_flatbuffers_message(
      schedule2::STUTTGART.name, schedule2::STUTTGART.eva,
      schedule2::KASSEL.name, schedule2::KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);
    auto const elements = rating::connection_to_graph_data::get_elements(
        *test_public_transport::schedule, *response->connections()->begin());
    REQUIRE(elements.size() == 2);

    distributions_container::precomputed_distributions_container
        precomputed_distributions(test_public_transport::schedule->node_count);
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    distributions_calculator::precomputation::perform_precomputation(
        *test_public_transport::schedule, s_t_distributions,
        precomputed_distributions);

    auto test_distributions =
        compute_test_distributions(precomputed_distributions);
    auto distributions = rate(elements, *test_public_transport::schedule,
                              precomputed_distributions);
    REQUIRE(distributions.size() == 4);
    REQUIRE(test_distributions.size() == 4);
    for (unsigned int i = 0; i < distributions.size(); i++) {
      REQUIRE(distributions[i] == test_distributions[i]);
    }
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
