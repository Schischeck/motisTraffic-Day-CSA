#include "gtest/gtest.h"

#include <string>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"

#include "motis/core/common/util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/message.h"
#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace flatbuffers;
using namespace motis::test;
using namespace motis::test::schedule;
using namespace motis::module;
using namespace motis::routing;

namespace motis {
namespace connectionchecker {

constexpr char const* kRoutingRequest = R""(
{
  "content_type": "RoutingRequest",
  "content": {
    "interval": {
      "begin": 1448368200,  // 2015-11-24 13:30:00 GMT+0100
      "end": 1448368200  // 2015-11-24 13:30:00 GMT+0100
    },
    "type": "OnTrip",
    "direction": "Forward",
    "path": [
      { "element_type": "StationPathElement",
        "element": {
          "eva_nr": "8000260",
          "name": ""
        }
      }, // Würzburg
      { "element_type": "StationPathElement",
        "element": {
          "eva_nr": "8000208",
          "name": ""
        }
      } // Köln-Ehrenfeld
    ],
    "additional_edges": []
  }
}
)"";

TEST(connectionchecker, finds_annotated_connections) {
  auto instance = launch_motis(kSimpleRealtimePath, kSimpleRealtimeDate,
                               {"routing", "connectionchecker", "realtime"});
  send(instance, get_simple_realtime_ris_message());
  auto resp = send(instance, make_msg(kRoutingRequest));
  auto journeys = message_to_journeys(resp->content<RoutingResponse const*>());

  ASSERT_EQ(1, journeys.size());
  auto j = journeys[0];

  // ice 628
  auto s0 = j.stops[0];  // Würzburg
  EXPECT_EQ(std::string("8000260"), s0.eva_no);
  EXPECT_EQ(0, s0.arrival.schedule_timestamp);
  EXPECT_EQ(0, s0.arrival.timestamp);
  // 2015-11-24 13:55:00 GMT+0100
  EXPECT_EQ(1448369700, s0.departure.schedule_timestamp);
  EXPECT_EQ(1448369700, s0.departure.timestamp);

  auto s1 = j.stops[1];  // Aschaffenburg
  // 2015-11-24 14:36:00 GMT+0100 --> 2015-11-24 14:37:00 GMT+0100
  EXPECT_EQ(1448372160, s1.departure.schedule_timestamp);
  EXPECT_EQ(1448372220, s1.departure.timestamp);

  auto s2 = j.stops[2];  // Frankfurt(Main)Hbf
  // 2015-11-24 15:04:00 GMT+0100 --> 2015-11-24 15:05:00 GMT+0100
  EXPECT_EQ(1448373840, s2.arrival.schedule_timestamp);
  EXPECT_EQ(1448373900, s2.arrival.timestamp);
  // 2015-11-24 15:10:00 GMT+0100 (back to normal)
  EXPECT_EQ(1448374200, s2.departure.schedule_timestamp);
  EXPECT_EQ(1448374200, s2.departure.timestamp);

  auto s3 = j.stops[3];  // Frankfurt(M) Flughafe
  // 2015-11-24 15:25:00 GMT+0100 --> 2015-11-24 15:30:00 GMT+0100
  EXPECT_EQ(1448375100, s3.departure.schedule_timestamp);
  EXPECT_EQ(1448375400, s3.departure.timestamp);

  // walk
  auto s4 = j.stops[4];  // Köln Messe/Deutz Gl.1
  // 2015-11-24 16:19:00 GMT+0100
  EXPECT_EQ(1448378340, s4.departure.schedule_timestamp);
  EXPECT_EQ(1448378340, s4.departure.timestamp);
  auto s5 = j.stops[5];  // Köln Messe/Deutz
  // 2015-11-24 16:20:00 GMT+0100
  EXPECT_EQ(1448378400, s5.arrival.schedule_timestamp);
  EXPECT_EQ(1448378400, s5.arrival.timestamp);

  // re 10958

  // 2015-11-24 16:33:00 GMT+0100
  EXPECT_EQ(1448379180, s5.departure.schedule_timestamp);
  EXPECT_EQ(1448379180, s5.departure.timestamp);

  auto s7 = j.stops[7];  // Köln-Ehrenfeld
  EXPECT_EQ(std::string("8000208"), s7.eva_no);
  // 2015-11-24 16:53:00 GMT+0100
  EXPECT_EQ(1448380260, s7.arrival.schedule_timestamp);
  EXPECT_EQ(1448380260, s7.arrival.timestamp);
}

}  // namespace connectionchecker
}  // namespace motis
