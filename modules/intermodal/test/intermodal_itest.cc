#include "gtest/gtest.h"

#include <string>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"

#include "geo/latlng.h"

#include "motis/core/common/util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/message.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace geo;
using namespace flatbuffers;
using namespace motis::osrm;
using namespace motis::test;
using namespace motis::test::schedule;
using namespace motis::module;
using namespace motis::routing;
using namespace motis::intermodal;
using motis::test::schedule::simple_realtime::dataset_opt;
using motis::test::schedule::simple_realtime::get_ris_message;

namespace motis {
namespace intermodal {

struct intermodal : public motis_instance_test {
  intermodal()
      : motis::test::motis_instance_test(dataset_opt,
                                         {"intermodal", "routing", "lookup"}) {
    instance_->register_op("/osrm/one_to_many", [](msg_ptr const& msg) {
      auto const req = motis_content(OSRMOneToManyRequest, msg);
      auto one = latlng{req->one()->lat(), req->one()->lng()};

      std::vector<Cost> costs;
      for (auto const& loc : *req->many()) {
        auto dist = distance(one, {loc->lat(), loc->lng()});
        costs.emplace_back(dist / WALK_SPEED, dist);
      }

      message_creator mc;
      mc.create_and_finish(
          MsgContent_OSRMOneToManyResponse,
          CreateOSRMOneToManyResponse(mc, mc.CreateVectorOfStructs(costs))
              .Union());
      return make_msg(mc);
    });
  }
};

TEST_F(intermodal, integration) {
  // auto const start_pos = Position{49.403567, 8.675442};  // Heidelberg Hbf
  // auto const dest_pos = latlng{49.681329, 8.616717};  // Bensheim
  //  Heidelberg Hbf -> Bensheim ( 2015-11-24 13:30:00 )
  auto json = R"(
    {
      "destination": {
        "type": "Module",
        "target": "/intermodal"
      },
      "content_type": "IntermodalRoutingRequest",
      "content": {
        "start_type": "IntermodalOntripStart",
        "start": {
          "position": { "lat": 49.403567, "lng": 8.675442},
          "departure_time": 1448368200
        },
        "start_modes": [{
          "mode_type": "Walk",
          "mode": { "max_duration": 60 }
        }],
        "destination": { "lat": 49.681329, "lng": 8.616717},
        "destination_modes":  [{
          "mode_type": "Walk",
          "mode": { "max_duration": 60 }
        }],
        "search_type": "SingleCriterion"
      }
    }
  )";

  auto res = call(make_msg(json));
  auto content = motis_content(IntermodalRoutingResponse, res);

  ASSERT_EQ(1, content->connections()->size());

  // auto journeys =
  // message_to_journeys(motis_content(IntermodalRoutingResponse, res));
}

// msg_ptr routing_request() const {
//   auto const interval = Interval(unix_time(1355), unix_time(1355));
//   message_creator mc;
//   mc.create_and_finish(
//       MsgContent_RoutingRequest,
//       CreateRoutingRequest(
//           mc, Start_PretripStart,
//           CreatePretripStart(mc,
//                              CreateInputStation(mc,
//                              mc.CreateString("8000260"),
//                                                 mc.CreateString("")),
//                              &interval)
//               .Union(),
//           CreateInputStation(mc, mc.CreateString("8000208"),
//                              mc.CreateString("")),
//           SearchType_SingleCriterion, SearchDir_Forward,
//           mc.CreateVector(std::vector<Offset<Via>>()),
//           mc.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()))
//           .Union(),
//       "/routing");
//   return make_msg(mc);
// }

// TEST_F(intermodal, finds_annotated_connections) {
//   publish(get_ris_message(sched()));
//   publish(make_no_msg("/ris/system_time_changed"));
//   auto res = call(routing_request());
//   auto journeys = message_to_journeys(motis_content(RoutingResponse, res));

//   ASSERT_EQ(1, journeys.size());
//   auto j = journeys[0];

//   // ICE 628
//   auto s0 = j.stops_[0];  // Wuerzburg
//   EXPECT_EQ("8000260", s0.eva_no_);
//   EXPECT_EQ(0, s0.arrival_.schedule_timestamp_);
//   EXPECT_EQ(0, s0.arrival_.timestamp_);
//   EXPECT_EQ(unix_time(1355), s0.departure_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1355), s0.departure_.timestamp_);

//   auto s1 = j.stops_[1];  // Aschaffenburg
//   EXPECT_EQ(unix_time(1436), s1.departure_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1437), s1.departure_.timestamp_);

//   auto s2 = j.stops_[2];  // Frankfurt(Main)Hbf
//   EXPECT_EQ(unix_time(1504), s2.arrival_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1505), s2.arrival_.timestamp_);
//   EXPECT_EQ(unix_time(1510), s2.departure_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1510), s2.departure_.timestamp_);

//   auto s3 = j.stops_[3];  // Frankfurt(M) Flughafe
//   EXPECT_EQ(unix_time(1525), s3.departure_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1530), s3.departure_.timestamp_);

//   // walk
//   auto s4 = j.stops_[4];  // Koeln Messe/Deutz Gl.1
//   EXPECT_EQ(unix_time(1619), s4.departure_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1619), s4.departure_.timestamp_);

//   auto s5 = j.stops_[5];  // Koeln Messe/Deutz
//   EXPECT_EQ(unix_time(1620), s5.arrival_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1620), s5.arrival_.timestamp_);

//   // RE 10958
//   EXPECT_EQ(unix_time(1633), s5.departure_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1633), s5.departure_.timestamp_);

//   auto s7 = j.stops_[7];  // Koeln-Ehrenfeld
//   EXPECT_EQ("8000208", s7.eva_no_);
//   EXPECT_EQ(unix_time(1651), s7.arrival_.schedule_timestamp_);
//   EXPECT_EQ(unix_time(1651), s7.arrival_.timestamp_);
// }

}  // namespace intermodal
}  // namespace motis
