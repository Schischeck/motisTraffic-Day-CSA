#include "gtest/gtest.h"

#include <vector>

#include "motis/core/schedule/edges.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/module.h"
#include "motis/loader/loader.h"

#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/schedules/schedule_hotels.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {

using namespace routing;

class reliability_late_connections : public test_motis_setup {
public:
  reliability_late_connections()
      : test_motis_setup(schedule_hotels::PATH, schedule_hotels::DATE) {}
};
class reliability_hotels_foot : public test_motis_setup {
public:
  reliability_hotels_foot()
      : test_motis_setup(schedule_hotels::PATH_FOOT_SCHEDULE,
                         schedule_hotels::DATE) {}
};

module::msg_ptr to_request(
    intermodal::individual_modes_container const& container) {
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  return b
      .add_pretrip_start(schedule_hotels::DARMSTADT.name_,
                         schedule_hotels::DARMSTADT.eva_,
                         1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */,
                         1445295000 /* 10/20/2015, 00:50:00 GMT+2:00 DST */)
      .add_destination(schedule_hotels::FRANKFURT.name_,
                       schedule_hotels::FRANKFURT.eva_)
      .add_additional_edges(container)
      .build_routing_request();
}

TEST_F(reliability_late_connections, search) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels::LANGEN.eva_, 360, 540, 5000);
  container.hotels_.emplace_back(schedule_hotels::OFFENBACH.eva_, 360, 540,
                                 5000);
  container.hotels_.emplace_back(schedule_hotels::MAINZ.eva_, 360, 540, 5000);

  container.taxi_.emplace_back(schedule_hotels::LANGEN.eva_,
                               schedule_hotels::FRANKFURT.eva_, 55, 6000, 1140,
                               60);
  /* this taxi should not be found
   * since it can only be reached after a hotel */
  container.taxi_.emplace_back(schedule_hotels::NEUISENBURG.eva_,
                               schedule_hotels::FRANKFURT.eva_, 10, 500, 1140,
                               60);
  /* this taxi should not be found
   * since it can only be reached after 3 o'clock */
  container.taxi_.emplace_back(schedule_hotels::WALLDORF.eva_,
                               schedule_hotels::FRANKFURT.eva_, 10, 500, 1140,
                               60);

  auto res = call(to_request(container));

  auto journeys = message_to_journeys(motis_content(RoutingResponse, res));

  ASSERT_EQ(4, journeys.size());

  struct {
    bool operator()(journey const& a, journey const& b) {
      return a.stops_.back().arrival_.timestamp_ <
             b.stops_.back().arrival_.timestamp_;
    }
  } journey_cmp;
  std::sort(journeys.begin(), journeys.end(), journey_cmp);

  { /* taxi connection, arrival 01:00 */
    auto const& j = journeys[0];
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(2, j.transports_.size());
    ASSERT_EQ(2, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::TAXI, j.transports_[1].slot_);
    ASSERT_EQ(6000, j.transports_[1].mumo_price_);
    ASSERT_EQ(schedule_hotels::LANGEN.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[2].eva_no_);
  }
  { /* direct connection, arrival 02:00 */
    auto const& j = journeys[1];
    ASSERT_EQ(60, j.night_penalty_);
    ASSERT_EQ(1, j.transports_.front().train_nr_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[1].eva_no_);
  }
  { /* hotel, arrival 10:45, 30 minutes night-penalty */
    auto const& j = journeys[2];
    ASSERT_EQ(35, j.night_penalty_);
    ASSERT_EQ(4, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
    ASSERT_EQ(5000, j.transports_[1].mumo_price_);
    ASSERT_EQ(5, j.transports_[2].train_nr_);
    ASSERT_EQ(schedule_hotels::OFFENBACH.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::OFFENBACH.eva_, j.stops_[2].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[3].eva_no_);
  }
  { /* hotel, arrival 10:50, no night-penalty */
    auto const& j = journeys[3];
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(2, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
    ASSERT_EQ(5000, j.transports_[1].mumo_price_);
    ASSERT_EQ(3, j.transports_[2].train_nr_);
    ASSERT_TRUE(j.transports_.back().is_walk_);
    ASSERT_EQ(schedule_hotels::LANGEN.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::LANGEN.eva_, j.stops_[2].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[3].eva_no_);
  }

  /* note: connection with hotel (Mainz)
   * and arrival 10:51 will be dominated.
   * Connection via Neu-Isenburg will not be delivered
   * since using taxi after hotel is not allowed. */
}

TEST_F(reliability_late_connections, taxi_not_allowed) {
  intermodal::individual_modes_container container;
  /* this taxi should not be found
   * since it can only be reached after 3 o'clock */
  container.taxi_.emplace_back(schedule_hotels::WALLDORF.eva_,
                               schedule_hotels::FRANKFURT.eva_, 10, 500, 1140,
                               60);
  auto msg = call(to_request(container));
  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));

  /* Whether the search finds a connection or not depends on
   * the filter in label.h: If the maximum travel time is limited,
   * the search finds nothing. Otherwise the search waits for a long time
   * at a station and takes the next train. */
  ASSERT_GE(1, journeys.size());
  if (journeys.size() == 1) {
    auto const& j = journeys.front();
    for (auto t : j.transports_) {
      ASSERT_FALSE(t.is_walk_); /* taxi not allowed */
    }
  }
}

TEST_F(reliability_hotels_foot, hotels_after_foot) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels::NEUISENBURG.eva_, 8 * 60,
                                 9 * 60, 5000);
  auto msg = call(to_request(container));

  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));
  ASSERT_EQ(1, journeys.size());

  auto const& j = journeys.back();
  ASSERT_EQ(6, j.transports_.size());
  ASSERT_TRUE(j.transports_[2].is_walk_);
  ASSERT_TRUE(j.transports_[3].is_walk_);
  ASSERT_EQ(intermodal::WALK, j.transports_[2].slot_);
  ASSERT_EQ(intermodal::HOTEL, j.transports_[3].slot_);
}

TEST_F(reliability_hotels_foot, foot_after_hotel) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels::LANGEN.eva_, 360, 540, 5000);
  auto msg = call(to_request(container));

  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));
  ASSERT_EQ(1, journeys.size());

  auto const& j = journeys.back();
  ASSERT_EQ(5, j.transports_.size());

  ASSERT_TRUE(j.transports_[1].is_walk_);
  ASSERT_TRUE(j.transports_[2].is_walk_);
  ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
  ASSERT_EQ(intermodal::WALK, j.transports_[2].slot_);
}

TEST_F(reliability_late_connections, DISABLED_late_conn_req) {
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  auto req =
      b.add_pretrip_start(schedule_hotels::DARMSTADT.name_,
                          schedule_hotels::DARMSTADT.eva_,
                          1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */,
                          1445298000 /* 10/20/2015, 01:40:00 GMT+2:00 DST */)
          .add_destination(schedule_hotels::FRANKFURT.name_,
                           schedule_hotels::FRANKFURT.eva_)
          .build_late_connection_request(50000 /* 50 km*/);

  using routing::RoutingResponse;
  auto const res = motis_content(ReliabilityRatingResponse, call(req));

  ASSERT_EQ(2, res->response()->connections()->size());
  ASSERT_EQ(2, (*res->response()->connections())[0]->transports()->size());
  ASSERT_EQ(
      Move_Walk,
      (*(*res->response()->connections())[0]->transports())[1]->move_type());
  auto taxi = reinterpret_cast<Walk const*>(
      (*(*res->response()->connections())[0]->transports())[1]->move());
  ASSERT_EQ("Taxi", std::string(taxi->mumo_type()->c_str()));

  ASSERT_EQ(2, (*res->response()->connections())[1]->transports()->size());
  ASSERT_EQ(
      Move_Transport,
      (*(*res->response()->connections())[1]->transports())[0]->move_type());
  auto direct_conn = reinterpret_cast<Transport const*>(
      (*(*res->response()->connections())[1]->transports())[0]->move());
  ASSERT_EQ(1, direct_conn->train_nr());
}

}  // namespace reliability
}  // namespace motis
