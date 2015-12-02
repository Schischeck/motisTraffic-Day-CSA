#include "gtest/gtest.h"

#include <vector>

#include "motis/core/common/date_util.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/core/schedule/edges.h"

#include "motis/loader/loader.h"

#include "motis/module/module.h"

#include "motis/routing/additional_edges.h"

/* todo: remove these dependencies on reliability module */
#include "../../reliability/test/include/test_schedule_setup.h"

namespace motis {
namespace routing {

class routing_late_connections : public reliability::test_motis_setup {
public:
  struct hotel_info {
    hotel_info(std::string const st, uint16_t earliest_checkout = 8 * 60,
               uint16_t min_stay_duration = 9 * 60, uint16_t price = 5000)
        : station_(st),
          earliest_checkout_(earliest_checkout),
          min_stay_duration_(min_stay_duration),
          price_(price) {}
    std::string station_;
    uint16_t earliest_checkout_;
    uint16_t min_stay_duration_;
    uint16_t price_;
  };
  struct taxi_info {
    taxi_info(std::string const from_st, std::string const to_st,
              uint16_t duration, uint16_t price)
        : from_station_(from_st),
          to_station_(to_st),
          duration_(duration),
          price_(price) {}
    std::string from_station_;
    std::string to_station_;
    uint16_t duration_;
    uint16_t price_;
  };

  routing_late_connections()
      : test_motis_setup("modules/reliability/resources/schedule_hotels/",
                         "20151019") {}

  std::string const DARMSTADT = "3333333";
  std::string const FRANKFURT = "1111111";
  std::string const LANGEN = "2222222";
  std::string const MAINZ = "3953754";
  std::string const NEUISENBURG = "5345291";
  std::string const OFFENBACH = "9727248";
  std::string const WALLDORF = "2813399";

  module::msg_ptr to_routing_msg(std::string const from, std::string const to,
                                 std::vector<hotel_info> const& hotel_infos,
                                 std::vector<taxi_info> const taxi_infos) {
    using namespace flatbuffers;
    module::MessageCreator b;
    std::vector<Offset<StationPathElement>> station_elements;
    station_elements.push_back(
        CreateStationPathElement(b, b.CreateString(""), b.CreateString(from)));
    station_elements.push_back(
        CreateStationPathElement(b, b.CreateString(""), b.CreateString(to)));
    Interval interval(
        motis_to_unixtime(motis::to_unix_time(2015, 10, 19), 1440 - 10),
        motis_to_unixtime(motis::to_unix_time(2015, 10, 19), 1440 + 50));
    std::vector<flatbuffers::Offset<AdditionalEdgeWrapper>> additional_edges;
    for (auto const& info : hotel_infos) {
      additional_edges.push_back(CreateAdditionalEdgeWrapper(
          b, AdditionalEdge_HotelEdge,
          CreateHotelEdge(b, b.CreateString(info.station_),
                          info.earliest_checkout_, info.min_stay_duration_,
                          info.price_)
              .Union()));
    }
    for (auto const& info : taxi_infos) {
      additional_edges.push_back(CreateAdditionalEdgeWrapper(
          b, AdditionalEdge_TimeDependentMumoEdge,
          CreateTimeDependentMumoEdge(
              b, CreateMumoEdge(b, b.CreateString(info.from_station_),
                                b.CreateString(info.to_station_),
                                info.duration_, info.price_),
              21 * 60, 3 * 60)
              .Union()));
    }
    b.CreateAndFinish(MsgContent_RoutingRequest,
                      CreateRoutingRequest(b, &interval, Type::Type_OnTrip,
                                           Direction::Direction_Forward,
                                           b.CreateVector(station_elements),
                                           b.CreateVector(additional_edges))
                          .Union());
    return module::make_msg(b);
  }
};

void test_hotel_edge(edge const& e, HotelEdge const* hotel_info,
                     schedule const& sched) {
  auto station_node =
      sched.station_nodes[sched.eva_to_station
                              .find(hotel_info->station_eva()->c_str())
                              ->second->index]
          .get();
  ASSERT_EQ(station_node->_id, e._from->_id);
  ASSERT_EQ(station_node->_id, e._to->_id);
  ASSERT_EQ(edge::HOTEL_EDGE, e.type());
  ASSERT_EQ(hotel_info->earliest_checkout_time(),
            e._m._hotel_edge._checkout_time);
  ASSERT_EQ(hotel_info->min_stay_duration(),
            e._m._hotel_edge._min_stay_duration);
  ASSERT_EQ(hotel_info->price(), e._m._hotel_edge._price);
}

TEST_F(routing_late_connections, test_hotel_edges) {
  std::vector<hotel_info> hotel_infos;
  hotel_infos.emplace_back(FRANKFURT, 7 * 60, 6 * 60, 5000);
  hotel_infos.emplace_back(LANGEN, 8 * 60, 9 * 60, 4000);
  auto message = to_routing_msg(DARMSTADT, FRANKFURT, hotel_infos, {});
  auto req = message->content<RoutingRequest const*>();
  auto hotel_edges =
      create_additional_edges(req->additional_edges(), get_schedule());

  ASSERT_EQ(2, hotel_edges.size());
  for (unsigned int i = 0; i < 2; ++i) {
    test_hotel_edge(
        hotel_edges[i],
        (HotelEdge const*)(*req->additional_edges())[i]->additional_edge(),
        get_schedule());
  }

  /* checkout-time: 7 * 60, min-stay-duration: 6 * 60 */
  {
    auto const cost = hotel_edges[0].get_edge_cost(23 * 60 + 59, nullptr);
    ASSERT_FALSE(cost.transfer);
    ASSERT_EQ(5000, cost.price);
    ASSERT_EQ(7 * 60 + 1, cost.time);
    ASSERT_EQ(6 * 60 + 10,
              hotel_edges[0].get_edge_cost(1440 + 50, nullptr).time);
    ASSERT_EQ(6 * 60, hotel_edges[0].get_edge_cost(1440 + 70, nullptr).time);
    ASSERT_EQ(6 * 60, hotel_edges[0].get_edge_cost(6 * 59, nullptr).time);
    ASSERT_EQ(1440, hotel_edges[0].get_edge_cost(7 * 60, nullptr).time);
  }

  /* checkout-time: 8 * 60, min-stay-duration: 9 * 60 */
  {
    auto const cost = hotel_edges[1].get_edge_cost(22 * 60 + 59, nullptr);
    ASSERT_FALSE(cost.transfer);
    ASSERT_EQ(4000, cost.price);
    ASSERT_EQ(9 * 60 + 1, cost.time);
    ASSERT_EQ(9 * 60, hotel_edges[1].get_edge_cost(23 * 60 + 1, nullptr).time);
  }

  ASSERT_EQ(5000, hotel_edges[0].get_minimum_cost().price);
  ASSERT_EQ(0, hotel_edges[0].get_minimum_cost().time);
  ASSERT_FALSE(hotel_edges[0].get_minimum_cost().transfer);
}

TEST_F(routing_late_connections, search) {
  std::vector<hotel_info> hotel_infos;
  hotel_infos.emplace_back(LANGEN);
  hotel_infos.emplace_back(OFFENBACH);
  hotel_infos.emplace_back(MAINZ);
  std::vector<taxi_info> taxi_infos;
  taxi_infos.emplace_back(LANGEN, FRANKFURT, 55, 6000);
  /* this taxi should not be found
   * since it can only be reached after a hotel */
  taxi_infos.emplace_back(NEUISENBURG, FRANKFURT, 10, 500);
  /* this taxi should not be found
   * since it can only be reached after 3 o'clock */
  taxi_infos.emplace_back(WALLDORF, FRANKFURT, 10, 500);
  auto req_msg = to_routing_msg(DARMSTADT, FRANKFURT, hotel_infos, taxi_infos);
  auto msg = bootstrap::send(motis_instance_, req_msg);

  ASSERT_NE(nullptr, msg);
  auto journeys = message_to_journeys(msg->content<RoutingResponse const*>());
  struct {
    bool operator()(journey const& a, journey const& b) {
      return a.stops.back().arrival.timestamp <
             b.stops.back().arrival.timestamp;
    }
  } journey_cmp;
  std::sort(journeys.begin(), journeys.end(), journey_cmp);
  ASSERT_EQ(4, journeys.size());
  { /* taxi connection, arrival 01:00 */
    auto const& j = journeys[0];
    ASSERT_EQ(0, j.night_penalty);
    ASSERT_EQ(3, j.transports.size());
    ASSERT_EQ(2, j.transports[0].train_nr);
    ASSERT_EQ(journey::transport::Mumo, j.transports[1].type);
    ASSERT_EQ("Taxi", j.transports[1].mumo_type_name);
    ASSERT_EQ(6000, j.transports[1].mumo_price);
    ASSERT_EQ(journey::transport::Walk, j.transports.back().type);
    ASSERT_EQ(LANGEN, j.stops[1].eva_no);
    ASSERT_EQ(FRANKFURT, j.stops[2].eva_no);
  }
  { /* direct connection, arrival 02:00 */
    auto const& j = journeys[1];
    ASSERT_EQ(60, j.night_penalty);
    ASSERT_EQ(1, j.transports.front().train_nr);
    ASSERT_EQ(FRANKFURT, j.stops[1].eva_no);
  }
  { /* hotel, arrival 10:45, 30 minutes night-penalty */
    auto const& j = journeys[2];
    ASSERT_EQ(35, j.night_penalty);
    ASSERT_EQ(4, j.transports[0].train_nr);
    ASSERT_EQ(journey::transport::Mumo, j.transports[1].type);
    ASSERT_EQ("Hotel", j.transports[1].mumo_type_name);
    ASSERT_EQ(5000, j.transports[1].mumo_price);
    ASSERT_EQ(5, j.transports[2].train_nr);
    ASSERT_EQ(OFFENBACH, j.stops[1].eva_no);
    ASSERT_EQ(OFFENBACH, j.stops[2].eva_no);
    ASSERT_EQ(FRANKFURT, j.stops[3].eva_no);
  }
  { /* hotel, arrival 10:50, no night-penalty */
    auto const& j = journeys[3];
    ASSERT_EQ(0, j.night_penalty);
    ASSERT_EQ(2, j.transports[0].train_nr);
    ASSERT_EQ(journey::transport::Mumo, j.transports[1].type);
    ASSERT_EQ("Hotel", j.transports[1].mumo_type_name);
    ASSERT_EQ(5000, j.transports[1].mumo_price);
    ASSERT_EQ(3, j.transports[2].train_nr);
    ASSERT_EQ(journey::transport::Walk, j.transports.back().type);
    ASSERT_EQ(LANGEN, j.stops[1].eva_no);
    ASSERT_EQ(LANGEN, j.stops[2].eva_no);
    ASSERT_EQ(FRANKFURT, j.stops[3].eva_no);
  }

  /* note: connection with hotel (Mainz)
   * and arrival 10:51 will be dominated.
   * Connection via Neu-Isenburg will not be delivered
   * since using taxi after hotel is not allowed. */
}

TEST_F(routing_late_connections, taxi_not_allowed) {
  std::vector<hotel_info> hotel_infos;
  std::vector<taxi_info> taxi_infos;
  /* this taxi should not be found
   * since it can only be reached after 3 o'clock */
  taxi_infos.emplace_back(WALLDORF, NEUISENBURG, 10, 500);
  auto req_msg =
      to_routing_msg(DARMSTADT, NEUISENBURG, hotel_infos, taxi_infos);
  auto msg = bootstrap::send(motis_instance_, req_msg);
  ASSERT_NE(nullptr, msg);
  ASSERT_EQ(0, (msg->content<RoutingResponse const*>())->connections()->size());
}

}  // namespace routing
}  // namespace motis
