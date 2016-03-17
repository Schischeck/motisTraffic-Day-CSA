#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis::test;
using namespace motis::test::schedule;

namespace motis {
namespace lookup {

constexpr auto kNotInPeriod = R""(
{ "content_type": "LookupStationEventsRequest",
  "content": { "eva_nr": "foo", "begin": 0, "end": 0 }}
)"";

constexpr auto kSiegenEmptyRequest = R""(
{ "content_type": "LookupStationEventsRequest",
  "content": {
    "eva_nr": "8000046",  // Siegen Hbf
    "begin": 1448373600,  // 2015-11-24 15:00:00 GMT+0100
    "end": 1448374200  // 2015-11-24 15:10:00 GMT+0100
  }}
)"";

constexpr auto kSiegenRequest = R""(
{ "content_type": "LookupStationEventsRequest",
  "content": {
    "eva_nr": "8000046",  // Siegen Hbf
    "begin": 1448373600,  // 2015-11-24 15:00:00 GMT+0100
    "end": 1448374260  // 2015-11-24 15:11:00 GMT+0100
  }}
)"";

constexpr auto kFrankfurtRequest = R""(
{ "content_type": "LookupStationEventsRequest",
  "content": {
    "eva_nr": "8000105",  // Frankfurt(Main)Hbf
    "begin": 1448371800,  // 2015-11-24 14:30:00 GMT+0100
    "end": 1448375400  // 2015-11-24 15:30:00 GMT+0100
  }}
)"";

TEST(lookup, station_events) {
  auto instance = launch_motis(kSimpleRealtimePath, kSimpleRealtimeDate,
                               {"lookup", "realtime"});
  send(instance, get_simple_realtime_ris_message());

  ASSERT_ANY_THROW(send(instance, make_msg(kNotInPeriod)));

  {
    auto msg = send(instance, make_msg(kSiegenEmptyRequest));
    ASSERT_EQ(MsgContent_LookupStationEventsResponse, msg->content_type());
    auto resp = msg->content<LookupStationEventsResponse const*>();
    ASSERT_EQ(0, resp->events()->size());  // end is exclusive
  }
  {
    auto msg = send(instance, make_msg(kSiegenRequest));
    ASSERT_EQ(MsgContent_LookupStationEventsResponse, msg->content_type());
    auto resp = msg->content<LookupStationEventsResponse const*>();
    ASSERT_EQ(1, resp->events()->size());

    auto event = resp->events()->Get(0);
    EXPECT_EQ(EventType_Departure, event->type());
    EXPECT_EQ(10958, event->train_nr());
    EXPECT_EQ(std::string(""), event->line_id()->str());
    EXPECT_EQ(1448374200, event->time());
    EXPECT_EQ(1448374200, event->schedule_time());

    auto trip_ids = event->trip_id();
    ASSERT_EQ(1, trip_ids->size());
    auto trip_id = trip_ids->Get(0);
    EXPECT_EQ(std::string("8000046"), trip_id->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, trip_id->type());
    EXPECT_EQ(10958, trip_id->train_nr());
    EXPECT_EQ(std::string(""), trip_id->line_id()->str());
    EXPECT_EQ(1448374200, trip_id->time());
  }
  {
    auto msg = send(instance, make_msg(kFrankfurtRequest));
    ASSERT_EQ(MsgContent_LookupStationEventsResponse, msg->content_type());
    auto resp = msg->content<LookupStationEventsResponse const*>();
    ASSERT_EQ(3, resp->events()->size());

    auto find_event = [resp](std::time_t schedule_time) -> StationEvent const* {
      for (auto const& e : *resp->events()) {
        if (e->schedule_time() == schedule_time) {
          return e;
        }
      }
      return nullptr;
    };

    auto e0 = find_event(1448372400);
    ASSERT_NE(nullptr, e0);
    EXPECT_EQ(EventType_Arrival, e0->type());
    EXPECT_EQ(2292, e0->train_nr());
    EXPECT_EQ(std::string("381"), e0->line_id()->str());
    EXPECT_EQ(1448372400, e0->time());
    EXPECT_EQ(1448372400, e0->schedule_time());

    auto tids0 = e0->trip_id();
    ASSERT_EQ(1, tids0->size());
    auto tid0 = tids0->Get(0);
    EXPECT_EQ(std::string("8000096"), tid0->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, tid0->type());
    EXPECT_EQ(2292, tid0->train_nr());
    EXPECT_EQ(std::string("381"), tid0->line_id()->str());
    EXPECT_EQ(1448366700, tid0->time());

    auto e1 = find_event(1448373840);
    ASSERT_NE(nullptr, e0);
    EXPECT_EQ(EventType_Arrival, e1->type());
    EXPECT_EQ(628, e1->train_nr());
    EXPECT_EQ(std::string(""), e1->line_id()->str());
    EXPECT_EQ(1448373900, e1->time());
    EXPECT_EQ(1448373840, e1->schedule_time());

    auto tids1 = e1->trip_id();
    ASSERT_EQ(1, tids1->size());
    auto tid1 = tids1->Get(0);
    EXPECT_EQ(std::string("8000261"), tid1->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, tid1->type());
    EXPECT_EQ(628, tid1->train_nr());
    EXPECT_EQ(std::string(""), tid1->line_id()->str());
    EXPECT_EQ(1448362440, tid1->time());

    auto e2 = find_event(1448374200);
    ASSERT_NE(nullptr, e0);
    EXPECT_EQ(EventType_Departure, e2->type());
    EXPECT_EQ(628, e2->train_nr());
    EXPECT_EQ(std::string(""), e2->line_id()->str());
    EXPECT_EQ(1448374200, e2->time());
    EXPECT_EQ(1448374200, e2->schedule_time());

    auto tids2 = e2->trip_id();
    ASSERT_EQ(1, tids2->size());
    auto tid2 = tids2->Get(0);
    EXPECT_EQ(std::string("8000261"), tid2->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, tid2->type());
    EXPECT_EQ(628, tid2->train_nr());
    EXPECT_EQ(std::string(""), tid2->line_id()->str());
    EXPECT_EQ(1448362440, tid2->time());
  }
}

}  // namespace lookup
}  // namespace motis
