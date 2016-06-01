#include "gtest/gtest.h"

#include <string>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"

#include "motis/core/common/util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/message.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace flatbuffers;
using namespace motis::test;
using namespace motis::test::schedule;
using namespace motis::module;
using namespace motis::routing;
using motis::test::schedule::simple_realtime::dataset_opt;
using motis::test::schedule::simple_realtime::get_ris_message;

namespace motis {
namespace routing {

struct routing_rt : public motis_instance_test {
  routing_rt()
      : motis::test::motis_instance_test(dataset_opt, {"routing", "rt"}) {}

  msg_ptr routing_request() const {
    auto const interval = Interval(unix_time(1330), unix_time(1330));
    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_RoutingRequest,
        CreateRoutingRequest(
            fbb, Start_PretripStart,
            CreatePretripStart(
                fbb, CreateInputStation(fbb, fbb.CreateString("8000260"),
                                        fbb.CreateString("")),
                &interval)
                .Union(),
            CreateInputStation(fbb, fbb.CreateString("8000208"),
                               fbb.CreateString("")),
            SearchType_SingleCriterionForward,
            fbb.CreateVector(std::vector<Offset<Via>>()),
            fbb.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()))
            .Union(),
        "/routing");
    return make_msg(fbb);
  }
};

TEST_F(routing_rt, finds_annotated_connections) {
  publish(get_ris_message(sched()));
  auto journeys = message_to_journeys(
      motis_content(RoutingResponse, call(routing_request())));

  ASSERT_EQ(1, journeys.size());
  auto j = journeys[0];

  // ice 628
  auto s0 = j.stops_[0];  // Würzburg
  EXPECT_EQ(std::string("8000260"), s0.eva_no_);
  EXPECT_EQ(0, s0.arrival_.schedule_timestamp_);
  EXPECT_EQ(0, s0.arrival_.timestamp_);
  // 2015-11-24 13:55:00 GMT+0100
  EXPECT_EQ(1448369700, s0.departure_.schedule_timestamp_);
  EXPECT_EQ(1448369700, s0.departure_.timestamp_);

  auto s1 = j.stops_[1];  // Aschaffenburg
  // 2015-11-24 14:36:00 GMT+0100 --> 2015-11-24 14:37:00 GMT+0100
  EXPECT_EQ(1448372160, s1.departure_.schedule_timestamp_);
  EXPECT_EQ(1448372220, s1.departure_.timestamp_);

  auto s2 = j.stops_[2];  // Frankfurt(Main)Hbf
  // 2015-11-24 15:04:00 GMT+0100 --> 2015-11-24 15:05:00 GMT+0100
  EXPECT_EQ(1448373840, s2.arrival_.schedule_timestamp_);
  EXPECT_EQ(1448373900, s2.arrival_.timestamp_);
  // 2015-11-24 15:10:00 GMT+0100 (back to normal)
  EXPECT_EQ(1448374200, s2.departure_.schedule_timestamp_);
  EXPECT_EQ(1448374200, s2.departure_.timestamp_);

  auto s3 = j.stops_[3];  // Frankfurt(M) Flughafe
  // 2015-11-24 15:25:00 GMT+0100 --> 2015-11-24 15:30:00 GMT+0100
  EXPECT_EQ(1448375100, s3.departure_.schedule_timestamp_);
  EXPECT_EQ(1448375400, s3.departure_.timestamp_);

  // walk
  auto s4 = j.stops_[4];  // Köln Messe/Deutz Gl.1
  // 2015-11-24 16:19:00 GMT+0100
  EXPECT_EQ(1448378340, s4.departure_.schedule_timestamp_);
  EXPECT_EQ(1448378340, s4.departure_.timestamp_);
  auto s5 = j.stops_[5];  // Köln Messe/Deutz
  // 2015-11-24 16:20:00 GMT+0100
  EXPECT_EQ(1448378400, s5.arrival_.schedule_timestamp_);
  EXPECT_EQ(1448378400, s5.arrival_.timestamp_);

  // re 10958

  // 2015-11-24 16:33:00 GMT+0100
  EXPECT_EQ(1448379180, s5.departure_.schedule_timestamp_);
  EXPECT_EQ(1448379180, s5.departure_.timestamp_);

  auto s7 = j.stops_[7];  // Köln-Ehrenfeld
  EXPECT_EQ(std::string("8000208"), s7.eva_no_);
  // 2015-11-24 16:53:00 GMT+0100
  EXPECT_EQ(1448380260, s7.arrival_.schedule_timestamp_);
  EXPECT_EQ(1448380260, s7.arrival_.timestamp_);
}

}  // namespace routing
}  // namespace motis
