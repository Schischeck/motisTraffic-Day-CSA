#include "gtest/gtest.h"

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/loader/loader.h"

#include "motis/module/dispatcher.h"

#include "motis/routing/routing.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"

using namespace motis::module;

namespace motis {
namespace reliability {

class reliability_test_rating : public test_motis_setup {
public:
  reliability_test_rating()
      : test_motis_setup("modules/reliability/resources/schedule2/",
                         "20150928") {}

  schedule_station const ERLANGEN = {"Erlangen", "0953067"};
  schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
  schedule_station const KASSEL = {"Kassel", "6380201"};
  schedule_station const STUTTGART = {"Stuttgart", "7309882"};
  short const ICE_S_E = 5;  // 11:32 --> 12:32
  short const ICE_E_K = 7;  // 12:45 --> 14:15
};

class reliability_test_cg : public test_motis_setup {
public:
  reliability_test_cg()
      : test_motis_setup("modules/reliability/resources/schedule7_cg/",
                         "20151019") {}

  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  short const RE_D_L = 1;  // 07:00 --> 07:10
  short const RE_L_F = 2;  // 07:15 --> 07:25
  short const S_L_F = 3;  // 07:16 --> 07:34
  short const IC_L_F = 4;  // 07:17 --> 07:40

  void test_journey(routing::Connection const* j,
                    std::string const departure_eva,
                    std::string const arrival_eva, time_t const departure_time,
                    time_t const arrival_time, unsigned int const train_nr) {
    auto const first_stop = j->stops()->begin();
    auto const last_stop = (*j->stops())[j->stops()->size() - 1];
    auto const transport =
        ((routing::Transport const*)j->transports()->begin()->move());
    ASSERT_EQ(departure_eva, first_stop->eva_nr()->c_str());
    ASSERT_EQ(arrival_eva, last_stop->eva_nr()->c_str());
    ASSERT_EQ(departure_time, first_stop->departure()->time());
    ASSERT_EQ(arrival_time, last_stop->arrival()->time());
    ASSERT_EQ(train_nr, transport->train_nr());
  }

  void test_alternative_rating(AlternativeRating const* rating,
                               time_t const dep_begin, time_t const arr_begin,
                               float const dep_first_prob,
                               float const arr_first_prob, float const sum) {
    ASSERT_EQ(dep_begin, rating->departure_distribution()->begin_time());
    ASSERT_FLOAT_EQ(dep_first_prob,
                    *rating->departure_distribution()->distribution()->begin());
    ASSERT_FLOAT_EQ(sum, rating->departure_distribution()->sum());
    ASSERT_EQ(arr_begin, rating->arrival_distribution()->begin_time());
    ASSERT_FLOAT_EQ(arr_first_prob,
                    *rating->arrival_distribution()->distribution()->begin());
    ASSERT_FLOAT_EQ(sum, rating->arrival_distribution()->sum());
  }

  void test_cg(motis::module::msg_ptr msg) {
    ASSERT_NE(nullptr, msg);

    ASSERT_EQ(MsgContent_ReliableRoutingResponse, msg->content_type());
    auto res = msg->content<ReliableRoutingResponse const*>();

    ASSERT_EQ(1, res->connection_graphs()->size());
    auto cg = *res->connection_graphs()->begin();
    ASSERT_EQ(3, cg->stops()->size());

    {
      auto const stop = (*cg->stops())[0];
      ASSERT_EQ(0, stop->index());
      ASSERT_EQ(1, stop->alternatives()->size());
      ASSERT_EQ(0, stop->alternatives()->begin()->journey());
      ASSERT_EQ(2, stop->alternatives()->begin()->next_stop());
      test_alternative_rating(stop->alternatives()->begin()->rating(),
                              1445238000, 1445238540, 0.8, 0.08, 1.0);
    }
    {
      auto const stop = (*cg->stops())[1];
      ASSERT_EQ(1, stop->index());
      ASSERT_EQ(0, stop->alternatives()->size());
    }
    {
      auto const stop = (*cg->stops())[2];
      ASSERT_EQ(2, stop->index());
      ASSERT_EQ(3, stop->alternatives()->size());
      {
        auto const alternative = (*stop->alternatives())[0];
        ASSERT_EQ(1, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating(stop->alternatives()->begin()->rating(),
                                1445238900, 1445239440, 0.592, 0.0592, 0.74);
      }
      {
        auto const alternative = (*stop->alternatives())[1];
        ASSERT_EQ(2, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating((*stop->alternatives())[1]->rating(),
                                1445238960, 1445239980, 0.192, 0.0192, 0.24);
      }
      {
        auto const alternative = (*stop->alternatives())[2];
        ASSERT_EQ(3, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating((*stop->alternatives())[2]->rating(),
                                1445239020, 1445240340, 0.016, 0.0016, 0.02);
      }
    }

    ASSERT_EQ(4, cg->journeys()->size());
    test_journey((*cg->journeys())[0], "3333333", "2222222",
                 1445238000 /* 07:00 */, 1445238600 /* 07:10 */, RE_D_L);
    test_journey((*cg->journeys())[1], "2222222", "1111111",
                 1445238900 /* 07:15 */, 1445239500 /* 07:25 */, RE_L_F);
    test_journey((*cg->journeys())[2], "2222222", "1111111",
                 1445238960 /* 07:16 */, 1445240040 /* 07:34 */, S_L_F);
    test_journey((*cg->journeys())[3], "2222222", "1111111",
                 1445239020 /* 07:17 */, 1445240400 /* 07:40 */, IC_L_F);
  }
};

TEST_F(reliability_test_rating, rating_request) {
  auto req_msg = flatbuffers::request_builder::request_builder()
                     .add_station(STUTTGART.name, STUTTGART.eva)
                     .add_station(KASSEL.name, KASSEL.eva)
                     .set_interval(std::make_tuple(28, 9, 2015),
                                   (motis::time)(11 * 60 + 32),
                                   (motis::time)(11 * 60 + 32))
                     .build_rating_request();
  auto msg = test::send(motis_instance_, req_msg);
  ASSERT_NE(nullptr, msg);

  auto response = msg->content<ReliabilityRatingResponse const*>();
  ASSERT_EQ(1, response->response()->connections()->size());

  ASSERT_NE(response->ratings(), nullptr);
  ASSERT_EQ(response->ratings()->size(), 1);
  auto const& rating = response->ratings()->begin();
  ASSERT_TRUE(equal(rating->connection_rating(), 1.0));
  auto const& rating_elements = *rating->rating_elements();
  ASSERT_EQ(rating_elements.size(), 2);
  ASSERT_TRUE(equal(rating_elements[0]->dep_distribution()->sum(), 1.0));
  ASSERT_TRUE(equal(rating_elements[0]->arr_distribution()->sum(), 1.0));
  ASSERT_TRUE(equal(rating_elements[1]->dep_distribution()->sum(), 1.0));
  ASSERT_TRUE(equal(rating_elements[1]->arr_distribution()->sum(), 1.0));
  ASSERT_NE((*rating->rating_elements())[0]->range(), nullptr);
  ASSERT_NE((*rating->rating_elements())[1]->range(), nullptr);

  ASSERT_NE(response->simple_ratings(), nullptr);
  ASSERT_EQ(response->simple_ratings()->size(), 1);
  auto const simple_rating = response->simple_ratings()->begin();
  ASSERT_TRUE(equal(simple_rating->connection_rating(), 0.995 * 0.995));
  ASSERT_EQ(simple_rating->rating_elements()->size(), 2);
  ASSERT_NE((*simple_rating->rating_elements())[0]->range(), nullptr);
  ASSERT_NE((*simple_rating->rating_elements())[1]->range(), nullptr);
  ASSERT_EQ((*simple_rating->rating_elements())[0]->ratings()->size(), 1);
  ASSERT_EQ((*simple_rating->rating_elements())[1]->ratings()->size(), 2);
}

TEST_F(reliability_test_cg, connection_tree) {
  auto req_msg =
      flatbuffers::request_builder::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(FRANKFURT.name, FRANKFURT.eva)
          .set_interval(std::make_tuple(19, 10, 2015), (motis::time)(7 * 60),
                        (motis::time)(7 * 60 + 1))
          .build_connection_tree_request(3, 1);
  auto msg = test::send(motis_instance_, req_msg);
  test_cg(msg);
}

TEST_F(reliability_test_cg, reliable_connection_graph) {
  auto req_msg =
      flatbuffers::request_builder::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(FRANKFURT.name, FRANKFURT.eva)
          .set_interval(std::make_tuple(19, 10, 2015), (motis::time)(7 * 60),
                        (motis::time)(7 * 60 + 1))
          .build_reliable_search_request(1);
  auto msg = test::send(motis_instance_, req_msg);
  test_cg(msg);
}

class reliability_late_connections : public test_motis_setup {
public:
  reliability_late_connections()
      : test_motis_setup("modules/reliability/resources/schedule_hotels/",
                         "20151019") {}

  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  schedule_station const OFFENBACH = {"Offenbach", "9727248"};
  schedule_station const MAINZ = {"Mainz", "3953754"};
};

/* taxi-info: from-station, duration, price */
using taxi_info = std::tuple<std::string, unsigned short, unsigned short>;

module::msg_ptr to_reliable_late_connections_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy,
    std::vector<taxi_info> const& taxi_infos) {
  using namespace routing;
  flatbuffers::request_builder::request_builder builder;
  builder.add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(ddmmyyyy, interval_begin, interval_end);

  auto& b = builder.b_;
  for (auto const& info : taxi_infos) {
    builder.add_additional_edge(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_TimeDependentMumoEdge,
        CreateTimeDependentMumoEdge(
            b, CreateMumoEdge(b, b.CreateString(std::get<0>(info)),
                              b.CreateString("-2") /* to dummy target */,
                              std::get<1>(info), std::get<2>(info)),
            21 * 60, 3 * 60, 1)
            .Union()));
  }

  return builder.build_late_connection_cequest();
}

TEST_F(reliability_late_connections, DISABLED_late_conn_req) {
  /* taxi-info: from-station, duration, price */
  std::vector<std::tuple<std::string, unsigned short, unsigned short>>
      taxi_infos;
  taxi_infos.emplace_back(LANGEN.eva, 55, 6000);

  auto req_msg = to_reliable_late_connections_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(23 * 60 + 50), (motis::time)(1500),
      std::make_tuple(19, 10, 2015), taxi_infos);
  auto msg = test::send(motis_instance_, req_msg);

  ASSERT_NE(nullptr, msg);
  auto response = msg->content<routing::RoutingResponse const*>();

  ASSERT_EQ(2, response->connections()->size());
  ASSERT_EQ(2, (*response->connections())[0]->transports()->size());
  auto taxi =
      (routing::Mumo const*)(*(*response->connections())[0]->transports())[1]
          ->move();
  ASSERT_EQ("Taxi", std::string(taxi->name()->c_str()));

  ASSERT_EQ(2, (*response->connections())[1]->transports()->size());
  auto direct_conn = (routing::Transport const*)(*(*response->connections())[1]
                                                      ->transports())[0]
                         ->move();
  ASSERT_EQ(1, direct_conn->train_nr());
}

}  // namespace reliability
}  // namespace motis
