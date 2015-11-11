#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"

#include "motis/reliability/probability_distribution.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure_interchange.h"

#include "motis/reliability/rating/connection_graph_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"

#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/connection_graph_search_tools.h"

#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "include/interchange_data_for_tests.h"
#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"

namespace motis {
namespace reliability {
using namespace search;
namespace rating {
namespace cg {
namespace test {

class test_connection_graph_rating_base : public test_schedule_setup {
public:
  test_connection_graph_rating_base(std::string schedule_name,
                                    std::time_t schedule_begin,
                                    std::time_t schedule_end)
      : test_schedule_setup(schedule_name, schedule_begin, schedule_end) {}

  std::pair<probability_distribution, probability_distribution>
  calc_distributions(interchange_data_for_tests const& ic_data,
                     probability_distribution const& arrival_distribution,
                     system_tools::setup& setup) {
    return calc_distributions(
        ic_data.arriving_light_conn_, ic_data.departing_light_conn_,
        ic_data.departing_route_edge_, arrival_distribution, setup);
  }

  std::pair<probability_distribution, probability_distribution>
  calc_distributions(light_connection const& arriving_light_conn,
                     light_connection const& departing_light_conn,
                     edge const& departing_route_edge,
                     probability_distribution const& arrival_distribution,
                     system_tools::setup& setup) {
    probability_distribution dep_dist, arr_dist;
    using namespace calc_departure_distribution;
    using namespace calc_arrival_distribution;
    interchange::compute_departure_distribution(
        data_departure_interchange(
            true, *departing_route_edge._from, departing_light_conn,
            arriving_light_conn, arrival_distribution, *schedule_,
            setup.reliability_module().precomputed_distributions(),
            setup.reliability_module().precomputed_distributions(),
            setup.reliability_module().s_t_distributions()),
        dep_dist);
    compute_arrival_distribution(
        data_arrival(*departing_route_edge._to, departing_light_conn, dep_dist,
                     *schedule_,
                     setup.reliability_module().s_t_distributions()),
        arr_dist);
    return std::make_pair(dep_dist, arr_dist);
  }
};

class test_connection_graph_rating : public test_connection_graph_rating_base {
public:
  test_connection_graph_rating()
      : test_connection_graph_rating_base(
            "modules/reliability/resources/schedule7_cg/",
            to_unix_time(2015, 10, 19), to_unix_time(2015, 10, 20)) {}

  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  short const RE_D_L = 1;  // 07:00 --> 07:10
  short const RE_L_F = 2;  // 07:15 --> 07:25
  short const S_L_F = 3;  // 07:16 --> 07:34
  short const IC_L_F = 4;  // 07:17 --> 07:40
};

class test_connection_graph_rating_foot
    : public test_connection_graph_rating_base {
public:
  test_connection_graph_rating_foot()
      : test_connection_graph_rating_base(
            "modules/reliability/resources/schedule3/",
            to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29)) {}
  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const MESSE = {"Frankfurt Messe", "2222222"};
  schedule_station const LANGEN = {"Langen", "3333333"};
  schedule_station const WEST = {"Frankfurt West", "4444444"};

  short const ICE_L_H = 1;  // 10:00 --> 10:10
  short const S_M_W = 2;  // 10:20 --> 10:25
};

TEST_F(test_connection_graph_rating, scheduled_transfer_filter) {
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
    ASSERT_TRUE(
        equal(0.0, scheduled_transfer_filter(
                       arr_dist, interchange_info(10, 10, 2, 0)).sum()));
  }
}

TEST_F(test_connection_graph_rating, compute_uncovered_arrival_distribution) {
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
                             arr_dist, interchange_info(10, 13, 1, 0)).sum()));
  ASSERT_TRUE(equal(0.0, compute_uncovered_arrival_distribution(
                             arr_dist, interchange_info(11, 13, 1, 1)).sum()));

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

/* rating of a cg consisting of a single journey with one interchange */
TEST_F(test_connection_graph_rating, single_connection) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_connection_tree_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), 1, 1, 15);
  bool test_cb_called = false;

  auto test_cb =
      [&](std::vector<std::shared_ptr<connection_graph> > const cgs) {
        test_cb_called = true;
        setup.ios_.stop();
        ASSERT_EQ(cgs.size(), 1);
        auto const cg = *cgs.front();
        ASSERT_EQ(3, cg.stops_.size());
        ASSERT_EQ(2, cg.journeys_.size());
        {
          connection_rating expected_rating_journey0;
          rating::rate(expected_rating_journey0, cg.journeys_[0],
                       *setup.reliability_context_);
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
              *schedule_, RE_D_L, RE_L_F, DARMSTADT.eva, LANGEN.eva,
              FRANKFURT.eva, 7 * 60, 7 * 60 + 10, 7 * 60 + 15, 7 * 60 + 25);
          auto const dists = calc_distributions(
              ic_data,
              detail::scheduled_transfer_filter(
                  cg.stops_[0]
                      .alternative_infos_.front()
                      .rating_.arrival_distribution_,
                  detail::interchange_info(7 * 60 + 10, 7 * 60 + 15, 5, 0)),
              setup);
          auto const& rating = cg.stops_[2].alternative_infos_.front().rating_;
          ASSERT_EQ(dists.first, rating.departure_distribution_);
          ASSERT_EQ(dists.second, rating.arrival_distribution_);
        }

        /* arrival distribution of the connection graph */
        probability_distribution exp_arr_dist;
        exp_arr_dist.init({0.0592, 0.4884, 0.1776, 0.0148}, 0);
        auto const cg_arr_dist = calc_arrival_distribution(cg);
        ASSERT_EQ(1445239440, cg_arr_dist.first);
        ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
      };

  boost::asio::io_service::work ios_work(setup.ios_);
  search_cgs(
      msg->content<ReliableRoutingRequest const*>(), setup.reliability_module(),
      0, std::make_shared<connection_graph_search::simple_optimizer>(1, 1, 15),
      test_cb);
  setup.ios_.run();
  ASSERT_TRUE(test_cb_called);
}

/* rating a cg with multiple alternatives */
TEST_F(test_connection_graph_rating, multiple_alternatives) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_connection_tree_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), 3, 1, 15);
  bool test_cb_called = false;

  auto test_cb = [&](
      std::vector<std::shared_ptr<connection_graph> > const cgs) {
    test_cb_called = true;
    setup.ios_.stop();
    ASSERT_EQ(1, cgs.size());
    auto const cg = *cgs.front();
    ASSERT_EQ(3, cg.stops_.size());
    ASSERT_EQ(4, cg.journeys_.size());

    {
      connection_rating expected_rating_journey0;
      rating::rate(expected_rating_journey0, cg.journeys_[0],
                   *setup.reliability_context_);
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
          *schedule_, RE_D_L, RE_L_F, DARMSTADT.eva, LANGEN.eva, FRANKFURT.eva,
          7 * 60, 7 * 60 + 10, 7 * 60 + 15, 7 * 60 + 25);
      auto const dists = calc_distributions(
          ic_data, detail::scheduled_transfer_filter(
                       uncovered_arr_dist, detail::interchange_info(
                                               7 * 60 + 10, 7 * 60 + 15, 5, 0)),
          setup);
      auto const& rating = cg.stops_[2].alternative_infos_[0].rating_;
      ASSERT_EQ(dists.first, rating.departure_distribution_);
      ASSERT_EQ(dists.second, rating.arrival_distribution_);
    }
    {
      /* note: S_L_F and RE_L_F are on the same route */
      auto const departing_lc =
          graph_accessor::get_departing_route_edge(
              *graph_accessor::get_first_route_node(*schedule_, RE_L_F))
              ->_m._route_edge._conns[1];
      interchange_data_for_tests ic_data(
          *schedule_, RE_D_L, RE_L_F, DARMSTADT.eva, LANGEN.eva, FRANKFURT.eva,
          7 * 60, 7 * 60 + 10, 7 * 60 + 15, 7 * 60 + 25);
      uncovered_arr_dist =
          rating::cg::detail::compute_uncovered_arrival_distribution(
              uncovered_arr_dist,
              detail::interchange_info(7 * 60 + 10, 7 * 60 + 15, 5, 0));
      auto const filtered_arr_dist = detail::scheduled_transfer_filter(
          uncovered_arr_dist,
          detail::interchange_info(7 * 60 + 10, 7 * 60 + 16, 5, 0));

      auto const dists = calc_distributions(
          ic_data.arriving_light_conn_, departing_lc,
          ic_data.departing_route_edge_, filtered_arr_dist, setup);
      auto const& rating = cg.stops_[2].alternative_infos_[1].rating_;
      ASSERT_EQ(dists.first, rating.departure_distribution_);
      ASSERT_EQ(dists.second, rating.arrival_distribution_);
    }
    {
      /* note: IC_L_F and RE_L_F are on the same route */
      auto const departing_lc =
          graph_accessor::get_departing_route_edge(
              *graph_accessor::get_first_route_node(*schedule_, RE_L_F))
              ->_m._route_edge._conns[2];
      interchange_data_for_tests ic_data(
          *schedule_, RE_D_L, RE_L_F, DARMSTADT.eva, LANGEN.eva, FRANKFURT.eva,
          7 * 60, 7 * 60 + 10, 7 * 60 + 15, 7 * 60 + 25);
      uncovered_arr_dist =
          rating::cg::detail::compute_uncovered_arrival_distribution(
              uncovered_arr_dist,
              detail::interchange_info(7 * 60 + 10, 7 * 60 + 16, 5, 0));
      auto const filtered_arr_dist = detail::scheduled_transfer_filter(
          uncovered_arr_dist,
          detail::interchange_info(7 * 60 + 10, 7 * 60 + 17, 5, 0));

      auto const dists = calc_distributions(
          ic_data.arriving_light_conn_, departing_lc,
          ic_data.departing_route_edge_, filtered_arr_dist, setup);
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
        {0.0592, 0.4884, 0.1776, 0.0148, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0192,
         0.1584, 0.0576, 0.0048, 0.0, 0.0, 0.0016, 0.0132, 0.0048, 0.0004},
        0);
    auto const cg_arr_dist = calc_arrival_distribution(cg);
    ASSERT_EQ(1445239440, cg_arr_dist.first);
    ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
  };

  boost::asio::io_service::work ios_work(setup.ios_);
  search_cgs(
      msg->content<ReliableRoutingRequest const*>(), setup.reliability_module(),
      0, std::make_shared<connection_graph_search::simple_optimizer>(3, 1, 15),
      test_cb);
  setup.ios_.run();
  ASSERT_TRUE(test_cb_called);
}

/* rating of a cg with a foot-path */
TEST_F(test_connection_graph_rating_foot, reliable_routing_request_foot) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_connection_tree_request(
      LANGEN.name, LANGEN.eva, WEST.name, WEST.eva, (motis::time)(10 * 60),
      (motis::time)(10 * 60), std::make_tuple(28, 9, 2015), 1, 1, 15);
  bool test_cb_called = false;

  auto test_cb = [&](
      std::vector<std::shared_ptr<connection_graph> > const cgs) {
    test_cb_called = true;
    setup.ios_.stop();
    ASSERT_EQ(cgs.size(), 1);
    auto const cg = *cgs.front();
    ASSERT_EQ(3, cg.stops_.size());
    ASSERT_EQ(2, cg.journeys_.size());
    {
      connection_rating expected_rating_journey0;
      rating::rate(expected_rating_journey0, cg.journeys_[0],
                   *setup.reliability_context_);
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
          *schedule_, ICE_L_H, S_M_W, LANGEN.eva, FRANKFURT.eva, MESSE.eva,
          WEST.eva, 10 * 60, 10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);

      auto const dists = calc_distributions(
          ic_data,
          detail::scheduled_transfer_filter(
              cg.stops_[0]
                  .alternative_infos_.front()
                  .rating_.arrival_distribution_,
              detail::interchange_info(10 * 60 + 10, 10 * 60 + 20, 10, 0)),
          setup);
      auto const& rating = cg.stops_[2].alternative_infos_.front().rating_;
      ASSERT_EQ(dists.first, rating.departure_distribution_);
      ASSERT_EQ(dists.second, rating.arrival_distribution_);
    }
    ASSERT_EQ(1443435900, cg.journeys_.back().stops.back().arrival.timestamp);

    /* arrival distribution of the connection graph */
    probability_distribution exp_arr_dist;
    exp_arr_dist.init({0.0592, 0.4884, 0.1776, 0.0148}, 0);
    auto const cg_arr_dist = calc_arrival_distribution(cg);
    ASSERT_EQ(1443435840, cg_arr_dist.first);
    ASSERT_EQ(exp_arr_dist, cg_arr_dist.second);
  };

  boost::asio::io_service::work ios_work(setup.ios_);
  search_cgs(
      msg->content<ReliableRoutingRequest const*>(), setup.reliability_module(),
      0, std::make_shared<connection_graph_search::simple_optimizer>(1, 1, 15),
      test_cb);
  setup.ios_.run();
  ASSERT_TRUE(test_cb_called);
}

}  // namespace test
}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
