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
    for (auto e : *resp->events()) {
      ASSERT_NE(nullptr, e);
      auto tids = e->trip_id();
      ASSERT_EQ(1, tids->size());
      auto tid = tids->Get(0);

      switch (e->schedule_time()) {
        case 1448372400:
          EXPECT_EQ(EventType_Arrival, e->type());
          EXPECT_EQ(2292, e->train_nr());
          EXPECT_EQ(std::string("381"), e->line_id()->str());
          EXPECT_EQ(1448372400, e->time());
          EXPECT_EQ(1448372400, e->schedule_time());

          EXPECT_EQ(std::string("8000096"), tid->eva_nr()->str());
          EXPECT_EQ(EventType_Departure, tid->type());
          EXPECT_EQ(2292, tid->train_nr());
          EXPECT_EQ(std::string("381"), tid->line_id()->str());
          EXPECT_EQ(1448366700, tid->time());
          break;

        case 1448373840:
          EXPECT_EQ(EventType_Arrival, e->type());
          EXPECT_EQ(628, e->train_nr());
          EXPECT_EQ(std::string(""), e->line_id()->str());
          EXPECT_EQ(1448373900, e->time());
          EXPECT_EQ(1448373840, e->schedule_time());

          EXPECT_EQ(std::string("8000261"), tid->eva_nr()->str());
          EXPECT_EQ(EventType_Departure, tid->type());
          EXPECT_EQ(628, tid->train_nr());
          EXPECT_EQ(std::string(""), tid->line_id()->str());
          EXPECT_EQ(1448362440, tid->time());
          break;

        case 1448374200:
          EXPECT_EQ(EventType_Departure, e->type());
          EXPECT_EQ(628, e->train_nr());
          EXPECT_EQ(std::string(""), e->line_id()->str());
          EXPECT_EQ(1448374200, e->time());
          EXPECT_EQ(1448374200, e->schedule_time());

          EXPECT_EQ(std::string("8000261"), tid->eva_nr()->str());
          EXPECT_EQ(EventType_Departure, tid->type());
          EXPECT_EQ(628, tid->train_nr());
          EXPECT_EQ(std::string(""), tid->line_id()->str());
          EXPECT_EQ(1448362440, tid->time());
          break;

        default: FAIL() << "unexpected event"; break;
      }
    }
  }
}

}  // namespace lookup
}  // namespace motis
