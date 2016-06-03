#include "gtest/gtest.h"

#include "motis/core/common/date_time_util.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/distributions/probability_distribution.h"

#include "motis/reliability/rating/cg_arrival_distribution.h"
#include "motis/reliability/rating/connection_graph_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"

#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/connection_graph_search_tools.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/module/context/motis_call.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/schedules/schedule3.h"
#include "../include/schedules/schedule7_cg.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
using namespace search;
namespace rating {
namespace cg {
namespace test {

class reliability_connection_graph_rating_base : public test_motis_setup {
public:
  reliability_connection_graph_rating_base(std::string schedule_name,
                                           std::string schedule_begin)
      : test_motis_setup(schedule_name, schedule_begin) {}

  std::pair<probability_distribution, probability_distribution>
  calc_distributions(interchange_data_for_tests const& ic_data,
                     probability_distribution const& arrival_distribution) {
    return calc_distributions(
        ic_data.arriving_light_conn_, ic_data.departing_light_conn_,
        *ic_data.arriving_route_edge_.to_, ic_data.departing_route_edge_,
        arrival_distribution);
  }

  std::pair<probability_distribution, probability_distribution>
  calc_distributions(light_connection const& arriving_light_conn,
                     light_connection const& departing_light_conn,
                     node const& arriving_route_node,
                     edge const& departing_route_edge,
                     probability_distribution const& arrival_distribution) {
    probability_distribution dep_dist, arr_dist;
    using namespace calc_departure_distribution;
    using namespace calc_arrival_distribution;
    interchange::compute_departure_distribution(
        data_departure_interchange(
            true, *departing_route_edge.from_, arriving_route_node,
            departing_light_conn, arriving_light_conn, arrival_distribution,
            get_reliability_module().precomputed_distributions(),
            get_reliability_module().precomputed_distributions().get_node(
                distributions_container::to_container_key(
                    *departing_route_edge.from_, departing_light_conn,
                    time_util::departure, get_schedule())),
            context(get_schedule(),
                    get_reliability_module().precomputed_distributions(),
                    get_reliability_module().s_t_distributions())),
        dep_dist);
    compute_arrival_distribution(
        data_arrival(*departing_route_edge.from_, *departing_route_edge.to_,
                     departing_light_conn, dep_dist, get_schedule(),
                     get_reliability_module().s_t_distributions()),
        arr_dist);
    return std::make_pair(dep_dist, arr_dist);
  }
};

class reliability_connection_graph_rating
    : public reliability_connection_graph_rating_base {
public:
  reliability_connection_graph_rating()
      : reliability_connection_graph_rating_base(schedule7_cg::PATH,
                                                 schedule7_cg::DATE) {}
};

class reliability_connection_graph_rating_foot
    : public reliability_connection_graph_rating_base {
public:
  reliability_connection_graph_rating_foot()
      : reliability_connection_graph_rating_base(schedule3::PATH,
                                                 schedule3::DATE) {}
};

TEST_F(reliability_connection_graph_rating, scheduled_transfer_filter) {
  using namespace detail;
  probability_distribution arr_dist;
  arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/

  ASSERT_EQ(arr_dist, scheduled_transfer_filter(
                          arr_dist, interchange_info(10, 13, 1, 0)));
  ASSERT_EQ(arr_dist, scheduled_transfer_filter(
                          arr_dist, interchange_info(10, 13, 2, 1)));

  {
    probability_distribution expected;
    expected.init({0.1, 0.2, 0.3}, -1);
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 13, 3, 1)));
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 12, 2, 1)));
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 13, 2, 0)));
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(9, 13, 3, 0)));
  }
  {
    ASSERT_TRUE(equal(
        0.0, scheduled_transfer_filter(arr_dist, interchange_info(10, 10, 2, 0))
                 .sum()));
  }
}

TEST_F(reliability_connection_graph_rating,
       scheduled_transfer_filter_realtime) {
  using namespace detail;
  {
    probability_distribution arr_dist;
    arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/
    probability_distribution expected;
    expected.init({0.1, 0.2}, -1);
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 11, 11, 12, false,
                                                       false, 1, 0)));
  }
  {
    probability_distribution arr_dist;
    arr_dist.init({1.0}, 1);
    probability_distribution expected;
    expected.init({0.0}, 1);
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 11, 11, 12, true,
                                                       false, 1, 0)));
  }
  {
    probability_distribution arr_dist;
    arr_dist.init({1.0}, 1);
    probability_distribution expected;
    expected.init({1.0}, 1);
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 11, 11, 12, true,
                                                       true, 1, 0)));
  }
  {
    probability_distribution arr_dist;
    arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/
    probability_distribution expected;
    expected.init({0.1, 0.2, 0.3}, -1);
    ASSERT_EQ(expected, scheduled_transfer_filter(
                            arr_dist, interchange_info(10, 11, 11, 12, false,
                                                       true, 1, 0)));
  }
}

TEST_F(reliability_connection_graph_rating,
       compute_uncovered_arrival_distribution) {
  using namespace detail;
  probability_distribution arr_dist;
  arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/

  ASSERT_EQ(arr_dist, compute_uncovered_arrival_distribution(
                          arr_dist, interchange_info(10, 10, 2, 0)));
  ASSERT_EQ(arr_dist, compute_uncovered_arrival_distribution(
                          arr_dist, interchange_info(10, 11, 3, 0)));
  ASSERT_EQ(arr_dist, compute_uncovered_arrival_distribution(
                          arr_dist, interchange_info(10, 10, 3, 1)));

  ASSERT_TRUE(equal(0.0, compute_uncovered_arrival_distribution(
                             arr_dist, interchange_info(10, 13, 1, 0))
                             .sum()));
  ASSERT_TRUE(equal(0.0, compute_uncovered_arrival_distribution(
                             arr_dist, interchange_info(11, 13, 1, 1))
                             .sum()));

  {
    probability_distribution expected;
    expected.init({0.3, 0.1}, 1);
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 12, 3, 1)));
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 12, 2, 0)));
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 13, 3, 0)));
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(9, 12, 3, 0)));
  }
}

TEST_F(reliability_connection_graph_rating,
       compute_uncovered_arrival_distribution_realtime) {
  using namespace detail;
  {
    probability_distribution arr_dist;
    arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/
    probability_distribution expected;
    expected.init({0.3, 0.1}, 1);
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 11, 11, 12, false,
                                                       false, 1, 0)));
  }
  {
    probability_distribution arr_dist;
    arr_dist.init({1.0}, 1);
    probability_distribution expected;
    expected.init({1.0}, 1);
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 11, 11, 12, true,
                                                       false, 1, 0)));
  }
  {
    probability_distribution arr_dist;
    arr_dist.init({1.0}, 1);
    probability_distribution expected; /* empty distribution */
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 11, 11, 12, true,
                                                       true, 1, 0)));
  }
  {
    probability_distribution arr_dist;
    arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/
    probability_distribution expected;
    expected.init({0.1}, 2);
    ASSERT_EQ(expected, compute_uncovered_arrival_distribution(
                            arr_dist, interchange_info(10, 11, 11, 12, false,
                                                       true, 1, 0)));
  }
}

/* rating of a cg consisting of a single journey with one interchange */
TEST_F(reliability_connection_graph_rating, single_connection) {
  auto msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule7_cg::DARMSTADT.name_,
                             schedule7_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 700))
          .add_destination(schedule7_cg::FRANKFURT.name_,
                           schedule7_cg::FRANKFURT.eva_)
          .build_connection_tree_request(1, 1);
  auto const cgs = run([&]() {
    return search_cgs(
        *motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(1, 1), 0);
  });

  ASSERT_EQ(1, cgs.size());
  auto const cg = *cgs.front();
  ASSERT_EQ(3, cg.stops_.size());
  ASSERT_EQ(2, cg.journeys_.size());
  {
    connection_rating expected_rating_journey0;
    rating::rate(expected_rating_journey0, cg.journeys_[0],
                 *reliability_context_);
    auto const& rating = cg.stops_[0].alternative_infos_.front().rating_;
    ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.front()
                  .departure_distribution_,
              rating.departure_distribution_);
    ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.back()
                  .arrival_distribution_,
              rating.arrival_distribution_);
  }
  {
    interchange_data_for_tests ic_data(
        get_schedule(), schedule7_cg::RE_D_L, schedule7_cg::RE_L_F,
        schedule7_cg::DARMSTADT.eva_, schedule7_cg::LANGEN.eva_,
        schedule7_cg::FRANKFURT.eva_, 7 * 60, 7 * 60 + 10, 7 * 60 + 15,
        7 * 60 + 25);
    auto const dists = calc_distributions(
        ic_data, detail::scheduled_transfer_filter(
                     cg.stops_[0]
                         .alternative_infos_.front()
                         .rating_.arrival_distribution_,
                     detail::interchange_info(7 * 60 + 10, 7 * 60 + 15, 5, 0)));
    auto const& rating = cg.stops_[2].alternative_infos_.front().rating_;
    ASSERT_EQ(dists.first, rating.departure_distribution_);
    ASSERT_EQ(dists.second, rating.arrival_distribution_);
  }

  /* arrival distribution of the connection graph */
  probability_distribution exp_arr_dist;
  exp_arr_dist.init({0.0592, 0.4884, 0.1776, 0.0148}, 0);
  auto const cg_arr_dist = calc_arrival_distribution(cg);
  ASSERT_EQ(1445232240 /* 10/19/2015, 7:24:00 AM GMT+2:00 DST */,
            cg_arr_dist.first);
  ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
}

/* rating a cg with multiple alternatives */
TEST_F(reliability_connection_graph_rating, multiple_alternatives) {
  auto msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule7_cg::DARMSTADT.name_,
                             schedule7_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 700))
          .add_destination(schedule7_cg::FRANKFURT.name_,
                           schedule7_cg::FRANKFURT.eva_)
          .build_connection_tree_request(3, 1);
  auto const cgs = run([&]() {
    return search_cgs(
        *motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(3, 1), 0);
  });

  ASSERT_EQ(1, cgs.size());
  auto const cg = *cgs.front();
  ASSERT_EQ(3, cg.stops_.size());
  ASSERT_EQ(4, cg.journeys_.size());

  {
    connection_rating expected_rating_journey0;
    rating::rate(expected_rating_journey0, cg.journeys_[0],
                 *reliability_context_);
    auto const& rating = cg.stops_[0].alternative_infos_.front().rating_;
    ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.front()
                  .departure_distribution_,
              rating.departure_distribution_);
    ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.back()
                  .arrival_distribution_,
              rating.arrival_distribution_);
  }
  auto uncovered_arr_dist =
      cg.stops_[0].alternative_infos_.front().rating_.arrival_distribution_;
  {
    interchange_data_for_tests ic_data(
        get_schedule(), schedule7_cg::RE_D_L, schedule7_cg::RE_L_F,
        schedule7_cg::DARMSTADT.eva_, schedule7_cg::LANGEN.eva_,
        schedule7_cg::FRANKFURT.eva_, 7 * 60, 7 * 60 + 10, 7 * 60 + 15,
        7 * 60 + 25);
    auto const dists = calc_distributions(
        ic_data, detail::scheduled_transfer_filter(
                     uncovered_arr_dist,
                     detail::interchange_info(7 * 60 + 10, 7 * 60 + 15, 5, 0)));
    auto const& rating = cg.stops_[2].alternative_infos_[0].rating_;
    ASSERT_EQ(dists.first, rating.departure_distribution_);
    ASSERT_EQ(dists.second, rating.arrival_distribution_);
  }
  {
    /* note: S_L_F and RE_L_F are on the same route */
    auto const departing_lc = graph_accessor::get_departing_route_edge(
                                  *graph_accessor::get_first_route_node(
                                      get_schedule(), schedule7_cg::RE_L_F))
                                  ->m_.route_edge_.conns_[1];
    interchange_data_for_tests ic_data(
        get_schedule(), schedule7_cg::RE_D_L, schedule7_cg::RE_L_F,
        schedule7_cg::DARMSTADT.eva_, schedule7_cg::LANGEN.eva_,
        schedule7_cg::FRANKFURT.eva_, 7 * 60, 7 * 60 + 10, 7 * 60 + 15,
        7 * 60 + 25);
    uncovered_arr_dist =
        rating::cg::detail::compute_uncovered_arrival_distribution(
            uncovered_arr_dist,
            detail::interchange_info(7 * 60 + 10, 7 * 60 + 15, 5, 0));
    auto const filtered_arr_dist = detail::scheduled_transfer_filter(
        uncovered_arr_dist,
        detail::interchange_info(7 * 60 + 10, 7 * 60 + 16, 5, 0));

    auto const dists =
        calc_distributions(ic_data.arriving_light_conn_, departing_lc,
                           *ic_data.arriving_route_edge_.to_,
                           ic_data.departing_route_edge_, filtered_arr_dist);
    auto const& rating = cg.stops_[2].alternative_infos_[1].rating_;
    ASSERT_EQ(dists.first, rating.departure_distribution_);
    ASSERT_EQ(dists.second, rating.arrival_distribution_);
  }
  {
    /* note: IC_L_F and RE_L_F are on the same route */
    auto const departing_lc = graph_accessor::get_departing_route_edge(
                                  *graph_accessor::get_first_route_node(
                                      get_schedule(), schedule7_cg::RE_L_F))
                                  ->m_.route_edge_.conns_[2];
    interchange_data_for_tests ic_data(
        get_schedule(), schedule7_cg::RE_D_L, schedule7_cg::RE_L_F,
        schedule7_cg::DARMSTADT.eva_, schedule7_cg::LANGEN.eva_,
        schedule7_cg::FRANKFURT.eva_, 7 * 60, 7 * 60 + 10, 7 * 60 + 15,
        7 * 60 + 25);
    uncovered_arr_dist =
        rating::cg::detail::compute_uncovered_arrival_distribution(
            uncovered_arr_dist,
            detail::interchange_info(7 * 60 + 10, 7 * 60 + 16, 5, 0));
    auto const filtered_arr_dist = detail::scheduled_transfer_filter(
        uncovered_arr_dist,
        detail::interchange_info(7 * 60 + 10, 7 * 60 + 17, 5, 0));

    auto const dists =
        calc_distributions(ic_data.arriving_light_conn_, departing_lc,
                           *ic_data.arriving_route_edge_.to_,
                           ic_data.departing_route_edge_, filtered_arr_dist);
    auto const& rating = cg.stops_[2].alternative_infos_[2].rating_;
    ASSERT_EQ(dists.first, rating.departure_distribution_);
    ASSERT_EQ(dists.second, rating.arrival_distribution_);

    uncovered_arr_dist =
        rating::cg::detail::compute_uncovered_arrival_distribution(
            uncovered_arr_dist,
            detail::interchange_info(7 * 60 + 10, 7 * 60 + 17, 5, 0));
    ASSERT_TRUE(equal(0.0, uncovered_arr_dist.sum()));
  }

  /* arrival distribution of the connection graph */
  probability_distribution exp_arr_dist;
  exp_arr_dist.init(
      {0.0592, 0.4884, 0.1776, 0.0148, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0192, 0.1584,
       0.0576, 0.0048, 0.0, 0.0, 0.0016, 0.0132, 0.0048, 0.0004},
      0);
  auto const cg_arr_dist = calc_arrival_distribution(cg);
  ASSERT_EQ(1445232240 /* 10/19/2015, 7:24:00 AM GMT+2:00 DST */,
            cg_arr_dist.first);
  ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
}

/* rating of a cg with a foot-path */
TEST_F(reliability_connection_graph_rating_foot,
       reliable_routing_request_foot) {
  auto msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule3::LANGEN.name_, schedule3::LANGEN.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 1000),
                             test_util::hhmm_to_unixtime(get_schedule(), 1000))
          .add_destination(schedule3::WEST.name_, schedule3::WEST.eva_)
          .build_connection_tree_request(1, 1);
  auto const cgs = run([&]() {
    return search_cgs(
        *motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(1, 1), 0);
  });

  ASSERT_EQ(1, cgs.size());
  auto const cg = *cgs.front();
  ASSERT_EQ(3, cg.stops_.size());
  ASSERT_EQ(2, cg.journeys_.size());
  {
    connection_rating expected_rating_journey0;
    rating::rate(expected_rating_journey0, cg.journeys_[0],
                 *reliability_context_);
    auto const& rating = cg.stops_[0].alternative_infos_.front().rating_;
    ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.front()
                  .departure_distribution_,
              rating.departure_distribution_);
    ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.back()
                  .arrival_distribution_,
              rating.arrival_distribution_);
  }
  {
    // arriving train ICE_L_H from Langen to Frankfurt
    // interchange at Frankfurt and walking to Messe
    // departing train S_M_W from Messe to West
    interchange_data_for_tests const ic_data(
        get_schedule(), schedule3::ICE_L_H, schedule3::S_M_W,
        schedule3::LANGEN.eva_, schedule3::FRANKFURT.eva_,
        schedule3::MESSE.eva_, schedule3::WEST.eva_, 10 * 60, 10 * 60 + 10,
        10 * 60 + 20, 10 * 60 + 25);

    auto const dists = calc_distributions(
        ic_data,
        detail::scheduled_transfer_filter(
            cg.stops_[0]
                .alternative_infos_.front()
                .rating_.arrival_distribution_,
            detail::interchange_info(10 * 60 + 10, 10 * 60 + 20, 10, 0)));
    auto const& rating = cg.stops_[2].alternative_infos_.front().rating_;
    ASSERT_EQ(dists.first, rating.departure_distribution_);
    ASSERT_EQ(dists.second, rating.arrival_distribution_);
  }
  ASSERT_EQ(1443428700, cg.journeys_.back().stops_.back().arrival_.timestamp_);

  /* arrival distribution of the connection graph */
  probability_distribution exp_arr_dist;
  exp_arr_dist.init({0.0592, 0.4884, 0.1776, 0.0148}, 0);
  auto const cg_arr_dist = calc_arrival_distribution(cg);
  ASSERT_EQ(1443428640, cg_arr_dist.first);
  ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
}

/* rating of a cg with a foot-path at the end of the journey */
TEST_F(reliability_connection_graph_rating_foot,
       reliable_routing_request_foot_at_the_end) {
  auto msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule3::LANGEN.name_, schedule3::LANGEN.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 1000),
                             test_util::hhmm_to_unixtime(get_schedule(), 1000))
          .add_destination(schedule3::MESSE.name_, schedule3::MESSE.eva_)
          .build_connection_tree_request(1, 1);
  auto const cgs = run([&]() {
    return search_cgs(
        *motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(1, 1), 0);
  });

  ASSERT_EQ(cgs.size(), 1);
  auto const cg = *cgs.front();
  ASSERT_EQ(2, cg.stops_.size());
  ASSERT_EQ(1, cg.journeys_.size());

  connection_rating expected_rating_journey0;
  rating::rate(expected_rating_journey0, cg.journeys_[0],
               *reliability_context_);
  auto const& rating = cg.stops_[0].alternative_infos_.front().rating_;
  ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.front()
                .departure_distribution_,
            rating.departure_distribution_);
  ASSERT_EQ(expected_rating_journey0.public_transport_ratings_.back()
                .arrival_distribution_,
            rating.arrival_distribution_);

  ASSERT_EQ(1443428400 /* 9/28/2015, 10:20:00 AM GMT+2:00 DST */,
            cg.journeys_.back().stops_.back().arrival_.timestamp_);

  /* arrival distribution of the connection graph */
  probability_distribution exp_arr_dist;
  exp_arr_dist.init({0.08, 0.66, 0.24, 0.02}, 0);
  auto const cg_arr_dist = calc_arrival_distribution(cg);
  ASSERT_EQ(1443428400 /* 9/28/2015, 10:20:00 AM GMT+2:00 DST */ +
                (cg.stops_[0]
                     .alternative_infos_.front()
                     .rating_.arrival_distribution_.first_minute() *
                 60),
            cg_arr_dist.first);
  ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
}

}  // namespace test
}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
