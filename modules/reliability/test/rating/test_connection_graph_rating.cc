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

class test_connection_graph_rating : public test_schedule_setup {
public:
  test_connection_graph_rating()
      : test_schedule_setup("modules/reliability/resources/schedule7_cg/",
                            to_unix_time(2015, 10, 19),
                            to_unix_time(2015, 10, 20)) {}

  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  short const RE_D_L = 1;  // 07:00 --> 07:10
  short const RE_L_F = 2;  // 07:15 --> 07:25
  short const S_L_F = 3;  // 07:16 --> 07:34
  short const IC_L_F = 4;  // 07:17 --> 07:40

  std::pair<probability_distribution, probability_distribution>
  calc_distributions(probability_distribution const& arrival_distribution,
                     system_tools::setup& setup) {
    interchange_data_for_tests ic_data(
        *schedule_, RE_D_L, RE_L_F, DARMSTADT.eva, LANGEN.eva, FRANKFURT.eva,
        7 * 60, 7 * 60 + 10, 7 * 60 + 15, 7 * 60 + 25);

    probability_distribution dep_dist, arr_dist;
    using namespace calc_departure_distribution;
    using namespace calc_arrival_distribution;
    interchange::compute_departure_distribution(
        data_departure_interchange(
            true, ic_data.tail_node_departing_train_,
            ic_data.departing_light_conn_, ic_data.arriving_light_conn_,
            arrival_distribution, *schedule_,
            setup.reliability_module().precomputed_distributions(),
            setup.reliability_module().precomputed_distributions(),
            setup.reliability_module().s_t_distributions()),
        dep_dist);
    compute_arrival_distribution(
        data_arrival(*ic_data.departing_route_edge_._to,
                     ic_data.departing_light_conn_, dep_dist, *schedule_,
                     setup.reliability_module().s_t_distributions()),
        arr_dist);
    return std::make_pair(dep_dist, arr_dist);
  }
};

TEST_F(test_connection_graph_rating, scheduled_transfer_filter) {
  using namespace detail;
  probability_distribution arr_dist;
  arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/

  ASSERT_EQ(arr_dist, scheduled_transfer_filter(arr_dist, 10, 13, 1, 0));
  ASSERT_EQ(arr_dist, scheduled_transfer_filter(arr_dist, 10, 13, 2, 1));

  {
    probability_distribution expected;
    expected.init({0.1, 0.2, 0.3}, -1);
    ASSERT_EQ(expected, scheduled_transfer_filter(arr_dist, 10, 13, 3, 1));
    ASSERT_EQ(expected, scheduled_transfer_filter(arr_dist, 10, 12, 2, 1));
    ASSERT_EQ(expected, scheduled_transfer_filter(arr_dist, 10, 13, 2, 0));
    ASSERT_EQ(expected, scheduled_transfer_filter(arr_dist, 9, 13, 3, 0));
  }
  {
    ASSERT_TRUE(
        equal(0.0, scheduled_transfer_filter(arr_dist, 10, 10, 2, 0).sum()));
  }
}

TEST_F(test_connection_graph_rating, compute_uncovered_arrival_distribution) {
  using namespace detail;
  probability_distribution arr_dist;
  arr_dist.init({0.1, 0.2, 0.3, 0.1}, -1); /* -1 ... 2*/

  ASSERT_EQ(arr_dist,
            compute_uncovered_arrival_distribution(arr_dist, 10, 10, 2, 0));
  ASSERT_EQ(arr_dist,
            compute_uncovered_arrival_distribution(arr_dist, 10, 11, 3, 0));
  ASSERT_EQ(arr_dist,
            compute_uncovered_arrival_distribution(arr_dist, 10, 10, 3, 1));

  ASSERT_TRUE(equal(0.0, compute_uncovered_arrival_distribution(
                             arr_dist, 10, 13, 1, 0).sum()));
  ASSERT_TRUE(equal(0.0, compute_uncovered_arrival_distribution(
                             arr_dist, 11, 13, 1, 1).sum()));

  {
    probability_distribution expected;
    expected.init({0.3, 0.1}, 1);
    ASSERT_EQ(expected,
              compute_uncovered_arrival_distribution(arr_dist, 10, 12, 3, 1));
    ASSERT_EQ(expected,
              compute_uncovered_arrival_distribution(arr_dist, 10, 12, 2, 0));
    ASSERT_EQ(expected,
              compute_uncovered_arrival_distribution(arr_dist, 10, 13, 3, 0));
    ASSERT_EQ(expected,
              compute_uncovered_arrival_distribution(arr_dist, 9, 12, 3, 0));
  }
}

TEST_F(test_connection_graph_rating, reliable_routing_request) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_reliable_routing_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), RequestType_ReliableSearch);
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
      auto const dists = calc_distributions(
          cg.stops_[0].alternative_infos_.front().rating_.arrival_distribution_,
          setup);
      auto const& rating = cg.stops_[2].alternative_infos_.front().rating_;
      ASSERT_EQ(dists.first, rating.departure_distribution_);
      ASSERT_EQ(dists.second, rating.arrival_distribution_);
    }
  };

  boost::asio::io_service::work ios_work(setup.ios_);
  connection_graph_search::simple_optimizer optimizer(1, 1, 15);
  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             setup.reliability_module(), 0, optimizer, test_cb);
  setup.ios_.run();
  ASSERT_TRUE(test_cb_called);
}

}  // namespace test
}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
