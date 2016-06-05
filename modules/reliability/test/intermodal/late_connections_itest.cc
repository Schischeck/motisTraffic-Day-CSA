#include "gtest/gtest.h"

#include <vector>

#include "motis/core/schedule/edges.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/module.h"
#include "motis/loader/loader.h"

#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/search/late_connections.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/schedules/schedule_hotels.h"
#include "../include/schedules/schedule_hotels_foot.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {

using namespace routing;

class reliability_late_connections : public test_motis_setup {
public:
  reliability_late_connections()
      : test_motis_setup(schedule_hotels::PATH, schedule_hotels::DATE, false,
                         false, "") {}
};
class reliability_hotels_foot : public test_motis_setup {
public:
  reliability_hotels_foot()
      : test_motis_setup(schedule_hotels_foot::PATH, schedule_hotels_foot::DATE,
                         false, false, "", true, schedule_hotels_foot::HOTELS) {
  }
};

module::msg_ptr to_request(
    intermodal::individual_modes_container const& container,
    bool const foot = false) {
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  return b
      .add_pretrip_start((foot ? schedule_hotels_foot::DARMSTADT.name_
                               : schedule_hotels::DARMSTADT.name_),
                         (foot ? schedule_hotels_foot::DARMSTADT.eva_
                               : schedule_hotels::DARMSTADT.eva_),
                         1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */,
                         1445295000 /* 10/20/2015, 00:50:00 GMT+2:00 DST */)
      .add_destination((foot ? schedule_hotels_foot::FRANKFURT.name_
                             : schedule_hotels::FRANKFURT.name_),
                       (foot ? schedule_hotels_foot::FRANKFURT.eva_
                             : schedule_hotels::FRANKFURT.eva_))
      .add_additional_edges(container)
      .build_routing_request();
}

TEST_F(reliability_late_connections, taxi_costs) {
  using taxi_cost = search::late_connections::detail::taxi_cost;
  auto const darmstadt = std::make_pair(49.872487, 8.630330);
  auto const frankfurt = std::make_pair(50.106439, 8.662156);
  {
    taxi_cost cost(darmstadt.first, darmstadt.second, frankfurt.first,
                   frankfurt.second, 250, 200, 10, 40, 100);
    ASSERT_EQ(36, cost.duration_);
    ASSERT_EQ(6817, cost.price_);
  }

  auto const tud = std::make_pair(49.877111, 8.655606);

  {
    taxi_cost cost(darmstadt.first, darmstadt.second, tud.first, tud.second,
                   250, 200, 10, 40, 100);
    ASSERT_EQ(14, cost.duration_);
    ASSERT_EQ(814, cost.price_);
  }
}

TEST_F(reliability_late_connections, search) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels::LANGEN.eva_, 360, 540, 5000);
  container.hotels_.emplace_back(schedule_hotels::OFFENBACH.eva_, 360, 540,
                                 5000);
  container.hotels_.emplace_back(schedule_hotels::MAINZ.eva_, 360, 540, 5001);

  container.taxis_.emplace_back(schedule_hotels::LANGEN.eva_,
                                schedule_hotels::FRANKFURT.eva_, 55, 6000, 1140,
                                60);
  /* this taxi should not be found
   * since it can only be reached after a hotel */
  container.taxis_.emplace_back(schedule_hotels::NEUISENBURG.eva_,
                                schedule_hotels::FRANKFURT.eva_, 10, 500, 1140,
                                60);
  /* this taxi should not be found
   * since it can only be reached after 3 o'clock */
  container.taxis_.emplace_back(schedule_hotels::WALLDORF.eva_,
                                schedule_hotels::FRANKFURT.eva_, 10, 500, 1140,
                                60);

  auto res = call(to_request(container));
  auto journeys = message_to_journeys(motis_content(RoutingResponse, res));
  ASSERT_EQ(5, journeys.size());

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
    ASSERT_EQ(6000, j.db_costs_);
    ASSERT_EQ(2, j.transports_.size());
    ASSERT_EQ(2, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::TAXI, j.transports_[1].slot_);
    // ASSERT_EQ(6000, j.transports_[1].mumo_price_);
    ASSERT_EQ(schedule_hotels::LANGEN.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[2].eva_no_);
  }
  { /* direct connection, arrival 02:00 */
    auto const& j = journeys[1];
    /* note: the connection ends at the station node and
     * the transfer time of 5 minutes adds 5 minutes penalty */
    ASSERT_EQ(65, j.night_penalty_);
    ASSERT_EQ(0, j.db_costs_);
    ASSERT_EQ(1, j.transports_.front().train_nr_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[1].eva_no_);
  }
  { /* hotel, departure 00:00, arrival 10:45, 35 minutes night-penalty */
    auto const& j = journeys[2];
    ASSERT_EQ(35, j.night_penalty_);
    ASSERT_EQ(5000, j.db_costs_);
    ASSERT_EQ(4, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
    // ASSERT_EQ(5000, j.transports_[1].mumo_price_);
    ASSERT_EQ(5, j.transports_[2].train_nr_);
    ASSERT_EQ(schedule_hotels::OFFENBACH.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::OFFENBACH.eva_, j.stops_[2].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[3].eva_no_);
  }
  { /* hotel, departure 23:50, arrival 10:50, no night-penalty */
    auto const& j = journeys[3];
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(5000, j.db_costs_);
    ASSERT_EQ(2, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
    // ASSERT_EQ(5000, j.transports_[1].mumo_price_);
    ASSERT_EQ(3, j.transports_[2].train_nr_);
    ASSERT_FALSE(j.transports_.back().is_walk_);
    ASSERT_EQ(schedule_hotels::LANGEN.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::LANGEN.eva_, j.stops_[2].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[3].eva_no_);
  }

  /* Journey (660, 1, 0)
     Darmstadt 21:51.5 --RE 6-> 22:10.5 Mainz
     Mainz 22:15.5 --3,0-> 07:15.6 Mainz
     Mainz 08:30.6 --ICE 7-> 08:51.6 Frankfurt */

  { /* hotel, departure 23:51, arrival 10:51, no night-penalty
       This connection survives because of alpha dominance */
    auto const& j = journeys[4];
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(5001, j.db_costs_);
    ASSERT_EQ(6, j.transports_[0].train_nr_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
    // ASSERT_EQ(5001, j.transports_[1].mumo_price_);
    ASSERT_EQ(7, j.transports_[2].train_nr_);
    ASSERT_FALSE(j.transports_.back().is_walk_);
    ASSERT_EQ(schedule_hotels::MAINZ.eva_, j.stops_[1].eva_no_);
    ASSERT_EQ(schedule_hotels::MAINZ.eva_, j.stops_[2].eva_no_);
    ASSERT_EQ(schedule_hotels::FRANKFURT.eva_, j.stops_[3].eva_no_);
  }

  /* note: Connection via Neu-Isenburg will not be delivered
   * since using taxi after hotel is not allowed. */
}

TEST_F(reliability_late_connections, taxi_not_allowed) {
  intermodal::individual_modes_container container;
  /* this taxi should not be found
   * since it can only be reached after 3 o'clock */
  container.taxis_.emplace_back(schedule_hotels::WALLDORF.eva_,
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
  container.hotels_.emplace_back(schedule_hotels_foot::NEUISENBURG.eva_, 480,
                                 540, 5000);
  auto msg = call(to_request(container, true));

  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));
  struct {
    bool operator()(journey const& a, journey const& b) {
      return a.db_costs_ < b.db_costs_;
    }
  } journey_cmp;
  std::sort(journeys.begin(), journeys.end(), journey_cmp);
  ASSERT_EQ(2, journeys.size());

  {
    auto const& j = journeys.front();
    ASSERT_EQ(3, j.transports_.size());
    ASSERT_EQ(4, j.stops_.size());
    ASSERT_EQ(300, j.night_penalty_);
    ASSERT_EQ(0, j.db_costs_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::WALK, j.transports_[1].slot_);
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
    ASSERT_TRUE(j.stops_[2].departure_.valid_);
    ASSERT_TRUE(j.stops_[3].arrival_.valid_);
  }
  {
    auto const& j = journeys.back();
    ASSERT_EQ(4, j.transports_.size());
    ASSERT_EQ(5, j.stops_.size());
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(5000, j.db_costs_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_TRUE(j.transports_[2].is_walk_);
    ASSERT_EQ(intermodal::WALK, j.transports_[1].slot_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[2].slot_);
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
    ASSERT_TRUE(j.stops_[2].departure_.valid_);
    ASSERT_TRUE(j.stops_[3].arrival_.valid_);
    ASSERT_TRUE(j.stops_[3].departure_.valid_);
    ASSERT_TRUE(j.stops_[4].arrival_.valid_);
  }
}

TEST_F(reliability_hotels_foot, foot_after_hotel) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels_foot::LANGEN.eva_, 360, 540,
                                 5000);
  auto msg = call(to_request(container, true));

  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));
  struct {
    bool operator()(journey const& a, journey const& b) {
      return a.db_costs_ < b.db_costs_;
    }
  } journey_cmp;
  std::sort(journeys.begin(), journeys.end(), journey_cmp);
  ASSERT_EQ(2, journeys.size());

  {
    auto const& j = journeys.front();
    ASSERT_EQ(3, j.transports_.size());
    ASSERT_EQ(4, j.stops_.size());
    ASSERT_EQ(300, j.night_penalty_);
    ASSERT_EQ(0, j.db_costs_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::WALK, j.transports_[1].slot_);
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
    ASSERT_TRUE(j.stops_[2].departure_.valid_);
    ASSERT_TRUE(j.stops_[3].arrival_.valid_);
  }
  {
    auto const& j = journeys.back();
    ASSERT_EQ(4, j.transports_.size());
    ASSERT_EQ(5, j.stops_.size());
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(5000, j.db_costs_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_TRUE(j.transports_[2].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);
    ASSERT_EQ(intermodal::WALK, j.transports_[2].slot_);
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
    ASSERT_TRUE(j.stops_[2].departure_.valid_);
    ASSERT_TRUE(j.stops_[3].arrival_.valid_);
    ASSERT_TRUE(j.stops_[3].departure_.valid_);
    ASSERT_TRUE(j.stops_[4].arrival_.valid_);
  }
}

TEST_F(reliability_hotels_foot, foot_after_hotel_at_beginning) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels_foot::LANGEN.eva_, 360, 540,
                                 5000);
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  auto req = b.add_ontrip_station_start(
                  schedule_hotels_foot::LANGEN.name_,
                  schedule_hotels_foot::LANGEN.eva_,
                  1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */)
                 .add_destination(schedule_hotels_foot::FRANKFURT.name_,
                                  schedule_hotels_foot::FRANKFURT.eva_)
                 .add_additional_edges(container)
                 .build_routing_request();
  auto msg = call(req);

  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));
  struct {
    bool operator()(journey const& a, journey const& b) {
      return a.db_costs_ < b.db_costs_;
    }
  } journey_cmp;
  std::sort(journeys.begin(), journeys.end(), journey_cmp);
  ASSERT_EQ(2, journeys.size());

  {
    auto const& j = journeys.front();
    ASSERT_EQ(2, j.transports_.size());
    ASSERT_EQ(300, j.night_penalty_);
    ASSERT_EQ(0, j.db_costs_);
    ASSERT_TRUE(j.transports_[0].is_walk_);
    ASSERT_EQ(intermodal::WALK, j.transports_[0].slot_);
    ASSERT_EQ(2, j.transports_[1].train_nr_);
  }
  {
    auto const& j = journeys.back();
    ASSERT_EQ(3, j.transports_.size());
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(5000, j.db_costs_);
    ASSERT_TRUE(j.transports_[0].is_walk_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[0].slot_);
    ASSERT_EQ(intermodal::WALK, j.transports_[1].slot_);
    ASSERT_EQ(2, j.transports_[2].train_nr_);

    ASSERT_EQ(4, j.stops_.size());
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
    ASSERT_TRUE(j.stops_[2].departure_.valid_);
    ASSERT_TRUE(j.stops_[3].arrival_.valid_);
  }
}

TEST_F(reliability_hotels_foot, hotels_after_foot_at_beginning) {
  intermodal::individual_modes_container container;
  container.hotels_.emplace_back(schedule_hotels_foot::NEUISENBURG.eva_, 480,
                                 540, 5000);
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  auto req = b.add_ontrip_station_start(
                  schedule_hotels_foot::LANGEN.name_,
                  schedule_hotels_foot::LANGEN.eva_,
                  1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */)
                 .add_destination(schedule_hotels_foot::FRANKFURT.name_,
                                  schedule_hotels_foot::FRANKFURT.eva_)
                 .add_additional_edges(container)
                 .build_routing_request();
  auto msg = call(req);

  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));
  struct {
    bool operator()(journey const& a, journey const& b) {
      return a.db_costs_ < b.db_costs_;
    }
  } journey_cmp;
  std::sort(journeys.begin(), journeys.end(), journey_cmp);
  ASSERT_EQ(2, journeys.size());

  {
    auto const& j = journeys.front();
    ASSERT_EQ(2, j.transports_.size());
    ASSERT_EQ(3, j.stops_.size());
    ASSERT_EQ(300, j.night_penalty_);
    ASSERT_EQ(0, j.db_costs_);
    ASSERT_TRUE(j.transports_[0].is_walk_);
    ASSERT_EQ(intermodal::WALK, j.transports_[0].slot_);
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
  }
  {
    auto const& j = journeys.back();
    ASSERT_EQ(3, j.transports_.size());
    ASSERT_EQ(4, j.stops_.size());
    ASSERT_EQ(0, j.night_penalty_);
    ASSERT_EQ(5000, j.db_costs_);
    ASSERT_TRUE(j.transports_[0].is_walk_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(intermodal::WALK, j.transports_[0].slot_);
    ASSERT_EQ(intermodal::HOTEL, j.transports_[1].slot_);

    ASSERT_EQ(4, j.stops_.size());
    ASSERT_TRUE(j.stops_[0].departure_.valid_);
    ASSERT_TRUE(j.stops_[1].arrival_.valid_);
    ASSERT_TRUE(j.stops_[1].departure_.valid_);
    ASSERT_TRUE(j.stops_[2].arrival_.valid_);
    ASSERT_TRUE(j.stops_[2].departure_.valid_);
    ASSERT_TRUE(j.stops_[3].arrival_.valid_);
  }
}

TEST_F(reliability_hotels_foot, late_conn_req_taxi) {
  journey dummy;
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  auto req =
      b.add_pretrip_start(schedule_hotels_foot::DARMSTADT.name_, "",
                          1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */,
                          1445298000 /* 10/20/2015, 01:40:00 GMT+2:00 DST */)
          .add_destination(schedule_hotels_foot::FRANKFURT.name_, "")
          .build_late_connection_request(50000 /* 50 km*/, dummy);
  auto const res_msg = call(req);
  auto const res = motis_content(ReliabilityRatingResponse, res_msg);

  ASSERT_EQ(3, res->response()->connections()->size());

  {
    auto const conn = (*res->response()->connections())[0];
    ASSERT_EQ(3567, conn->db_costs());
    ASSERT_EQ(0, conn->night_penalty());
    ASSERT_EQ(2, conn->transports()->size());
    {
      auto const move = (*conn->transports())[0];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ("RE 1", transport->name()->str());
      ASSERT_EQ(1, transport->train_nr());
    }
    {
      auto const move = (*conn->transports())[1];
      ASSERT_EQ(Move_Walk, move->move_type());
      auto const walk = reinterpret_cast<Walk const*>(move->move());
      ASSERT_EQ("Taxi", walk->mumo_type()->str());
    }
  }
  {
    auto const conn = (*res->response()->connections())[1];
    ASSERT_EQ(1967, conn->db_costs());
    ASSERT_EQ(0, conn->night_penalty());
    ASSERT_EQ(3, conn->transports()->size());
    {
      auto const move = (*conn->transports())[0];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ(1, transport->train_nr());
    }
    {
      auto const move = (*conn->transports())[1];
      ASSERT_EQ(Move_Walk, move->move_type());
      auto const walk = reinterpret_cast<Walk const*>(move->move());
      ASSERT_EQ("Walk", walk->mumo_type()->str());
    }
    {
      auto const move = (*conn->transports())[2];
      ASSERT_EQ(Move_Walk, move->move_type());
      auto const walk = reinterpret_cast<Walk const*>(move->move());
      ASSERT_EQ("Taxi", walk->mumo_type()->str());
    }
  }
  {
    auto const conn = (*res->response()->connections())[2];
    ASSERT_EQ(0, conn->db_costs());
    ASSERT_EQ(300, conn->night_penalty());
    ASSERT_EQ(3, conn->transports()->size());
    {
      auto const move = (*conn->transports())[0];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ(1, transport->train_nr());
    }
    {
      auto const move = (*conn->transports())[1];
      ASSERT_EQ(Move_Walk, move->move_type());
      auto const walk = reinterpret_cast<Walk const*>(move->move());
      ASSERT_EQ("Walk", walk->mumo_type()->str());
    }
    {
      auto const move = (*conn->transports())[2];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ(2, transport->train_nr());
    }
  }
}

TEST_F(reliability_hotels_foot, late_conn_req_hotel) {
  journey dummy;
  flatbuffers::request_builder b(SearchType_LateConnectionsForward);
  auto req =
      b.add_pretrip_start(schedule_hotels_foot::DARMSTADT.name_, "",
                          1445291400 /* 10/19/2015, 23:50:00 GMT+2:00 DST */,
                          1445298000 /* 10/20/2015, 01:40:00 GMT+2:00 DST */)
          .add_destination(schedule_hotels_foot::BERLIN.name_, "")
          .build_late_connection_request(50000 /* 50 km*/, dummy);
  auto const res_msg = call(req);
  auto const res = motis_content(ReliabilityRatingResponse, res_msg);

  ASSERT_EQ(2, res->response()->connections()->size());

  {
    auto const conn = (*res->response()->connections())[0];
    ASSERT_EQ(5000, conn->db_costs());
    ASSERT_EQ(0, conn->night_penalty());
    ASSERT_EQ(3, conn->transports()->size());
    {
      auto const move = (*conn->transports())[0];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ("RE 1", transport->name()->str());
      ASSERT_EQ(1, transport->train_nr());
    }
    {
      auto const move = (*conn->transports())[1];
      ASSERT_EQ(Move_Walk, move->move_type());
      auto const walk = reinterpret_cast<Walk const*>(move->move());
      ASSERT_EQ("Hotel", walk->mumo_type()->str());
    }
    {
      auto const move = (*conn->transports())[2];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ(3, transport->train_nr());
    }
  }
  {
    auto const conn = (*res->response()->connections())[1];
    ASSERT_EQ(0, conn->db_costs());
    ASSERT_EQ(300, conn->night_penalty());
    ASSERT_EQ(2, conn->transports()->size());
    {
      auto const move = (*conn->transports())[0];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ(1, transport->train_nr());
    }
    {
      auto const move = (*conn->transports())[1];
      ASSERT_EQ(Move_Transport, move->move_type());
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      ASSERT_EQ(3, transport->train_nr());
    }
  }
}

}  // namespace reliability
}  // namespace motis
