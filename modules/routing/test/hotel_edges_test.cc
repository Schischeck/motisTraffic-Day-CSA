#include "gtest/gtest.h"

#include <vector>

#include "motis/core/common/date_util.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/core/schedule/edges.h"

#include "motis/loader/loader.h"

#include "motis/module/module.h"

#include "motis/routing/hotel_edges.h"

/* TODO: remove these dependencies to reliability module */
#include "../../reliability/test/include/test_schedule_setup.h"
#include "motis/reliability/tools/system.h"

namespace motis {
namespace routing {

class routing_hotel_edges : public reliability::test_schedule_setup {
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
  routing_hotel_edges()
      : test_schedule_setup("modules/reliability/resources/schedule_hotels/",
                            to_unix_time(2015, 10, 19),
                            to_unix_time(2015, 10, 21)) {}

  std::string const FRANKFURT = "1111111";
  std::string const LANGEN = "2222222";
  std::string const DARMSTADT = "3333333";
  std::string const OFFENBACH = "9727248";
  std::string const MAINZ = "3953754";

  module::msg_ptr to_routing_msg(std::vector<hotel_info> const hotel_infos) {
    using namespace flatbuffers;
    module::MessageCreator b;
    std::vector<Offset<StationPathElement>> station_elements;
    station_elements.push_back(CreateStationPathElement(
        b, b.CreateString(""), b.CreateString(DARMSTADT)));
    station_elements.push_back(CreateStationPathElement(
        b, b.CreateString(""), b.CreateString(FRANKFURT)));
    Interval interval(
        motis_to_unixtime(motis::to_unix_time(2015, 10, 19), 1440 - 10),
        motis_to_unixtime(motis::to_unix_time(2015, 10, 19), 1440 + 50));
    std::vector<flatbuffers::Offset<HotelEdge>> f_hotel_infos;
    for (auto const& info : hotel_infos) {
      f_hotel_infos.push_back(CreateHotelEdge(
          b, b.CreateString(info.station_), info.earliest_checkout_,
          info.min_stay_duration_, info.price_));
    }
    b.CreateAndFinish(MsgContent_RoutingRequest,
                      CreateRoutingRequest(b, &interval, Type::Type_OnTrip,
                                           Direction::Direction_Forward,
                                           b.CreateVector(station_elements),
                                           b.CreateVector(f_hotel_infos))
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
  ASSERT_EQ(station_node, e._from);
  ASSERT_EQ(station_node, e._to);
  ASSERT_EQ(edge::HOTEL_EDGE, e.type());
  ASSERT_EQ(hotel_info->checkout_time(), e._m._hotel_edge._checkout_time);
  ASSERT_EQ(hotel_info->min_stay_duration(),
            e._m._hotel_edge._min_stay_duration);
  ASSERT_EQ(hotel_info->price(), e._m._hotel_edge._price);
}

TEST_F(routing_hotel_edges, test_costs) {
  std::vector<hotel_info> hotel_infos;
  hotel_infos.emplace_back(FRANKFURT, 7 * 60, 6 * 60, 5000);
  hotel_infos.emplace_back(LANGEN, 8 * 60, 9 * 60, 4000);
  auto message = to_routing_msg(hotel_infos);
  auto req = message->content<RoutingRequest const*>();
  auto hotel_edges = create_hotel_edges(req->hotel_edges(), *schedule_);

  ASSERT_EQ(2, hotel_edges.size());
  for (unsigned int i = 0; i < 2; ++i) {
    test_hotel_edge(hotel_edges[i], (*req->hotel_edges())[i], *schedule_);
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

TEST_F(routing_hotel_edges, search) {
  reliability::system_tools::setup setup(schedule_.get());
  bool test_cb_called = false;
  std::vector<hotel_info> hotel_infos;
  hotel_infos.emplace_back(LANGEN);
  hotel_infos.emplace_back(OFFENBACH);
  hotel_infos.emplace_back(MAINZ);
  auto msg = to_routing_msg(hotel_infos);

  std::cout << "\nSTATIONS" << std::endl;
  for (auto const& st : schedule_->stations) {
    std::cout << st->name << " ";
  }
  std::cout << std::endl;

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    test_cb_called = true;
    ASSERT_EQ(nullptr, e);
    ASSERT_NE(nullptr, msg);
    std::cout << "\nResult:\n" << msg->to_json() << std::endl;
    auto journeys = message_to_journeys(msg->content<RoutingResponse const*>());
    struct {
      bool operator()(journey const& a, journey const& b) {
        return a.stops.back().arrival.timestamp <
               b.stops.back().arrival.timestamp;
      }
    } journey_cmp;
    std::sort(journeys.begin(), journeys.end(), journey_cmp);
    ASSERT_EQ(3, journeys.size());
    { /* direct connection, arrival 02:00 */
      auto const& j = journeys[0];
      ASSERT_EQ(1, j.transports.front().train_nr);
      ASSERT_EQ(FRANKFURT, j.stops[1].eva_no);
    }
    { /* hotel, arrival 10:45, 30 minutes night-penalty */
      auto const& j = journeys[1];
      ASSERT_EQ(4, j.transports[0].train_nr);
      ASSERT_TRUE(j.transports[1].walk);
      ASSERT_EQ(5, j.transports[2].train_nr);
      ASSERT_EQ(OFFENBACH, j.stops[1].eva_no);
      ASSERT_EQ(OFFENBACH, j.stops[2].eva_no);
      ASSERT_EQ(FRANKFURT, j.stops[3].eva_no);
    }
    { /* hotel, arrival 10:50, no night-penalty */
      auto const& j = journeys[2];
      ASSERT_EQ(2, j.transports[0].train_nr);
      ASSERT_TRUE(j.transports[1].walk);
      ASSERT_EQ(3, j.transports[2].train_nr);
      ASSERT_EQ(LANGEN, j.stops[1].eva_no);
      ASSERT_EQ(LANGEN, j.stops[2].eva_no);
      ASSERT_EQ(FRANKFURT, j.stops[3].eva_no);
    }

    /* note: connection with hotel (Mainz)
     * and arrival 10:51 will be dominated */
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();

  ASSERT_TRUE(test_cb_called);
}

}  // namespace routing
}  // namespace motis
