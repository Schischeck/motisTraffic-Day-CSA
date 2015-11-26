#include "gtest/gtest.h"

#include <vector>

#include "motis/core/common/date_util.h"
#include "motis/core/schedule/edges.h"

#include "motis/loader/loader.h"

#include "motis/module/module.h"

#include "motis/routing/hotel_edges.h"

/* todo: remove this dependency to reliability module */
#include "../../reliability/test/include/test_schedule_setup.h"

namespace motis {
namespace routing {

class routing_hotel_edges : public reliability::test_schedule_setup {
public:
  routing_hotel_edges()
      : test_schedule_setup("modules/reliability/resources/schedule_hotels/",
                            to_unix_time(2015, 10, 19),
                            to_unix_time(2015, 10, 20)) {}

  std::string const FRANKFURT = "1111111";
  std::string const LANGEN = "2222222";
  std::string const DARMSTADT = "3333333";

  module::msg_ptr to_routing_msg() {
    using namespace flatbuffers;
    module::MessageCreator b;
    std::vector<Offset<StationPathElement>> station_elements;
    station_elements.push_back(
        CreateStationPathElement(b, b.CreateString(""), b.CreateString("")));
    station_elements.push_back(
        CreateStationPathElement(b, b.CreateString(""), b.CreateString("")));
    Interval interval(1, 1);
    std::vector<flatbuffers::Offset<HotelEdge>> hotel_infos;
    hotel_infos.push_back(
        CreateHotelEdge(b, b.CreateString(FRANKFURT), 7 * 60, 6 * 60, 5000));
    hotel_infos.push_back(
        CreateHotelEdge(b, b.CreateString(LANGEN), 8 * 60, 9 * 60, 4000));
    b.CreateAndFinish(MsgContent_RoutingRequest,
                      CreateRoutingRequest(b, &interval, Type::Type_OnTrip,
                                           Direction::Direction_Forward,
                                           b.CreateVector(station_elements),
                                           b.CreateVector(hotel_infos))
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
  auto message = to_routing_msg();
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

}  // namespace routing
}  // namespace motis
