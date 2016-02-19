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

TEST(lookup, finds_station_events) {
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

    auto id_event = event->id_event();
    EXPECT_EQ(std::string("8000046"), id_event->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, id_event->type());
    EXPECT_EQ(10958, id_event->train_nr());
    EXPECT_EQ(std::string(""), id_event->line_id()->str());
    EXPECT_EQ(1448374200, id_event->schedule_time());
  }
  {
    auto msg = send(instance, make_msg(kFrankfurtRequest));
    ASSERT_EQ(MsgContent_LookupStationEventsResponse, msg->content_type());
    auto resp = msg->content<LookupStationEventsResponse const*>();
    ASSERT_EQ(3, resp->events()->size());

    // arrivals, then departures
    // order depends on the implementation

    // two arrivals
    auto e0 = resp->events()->Get(0);
    auto e1 = resp->events()->Get(1);
    if(e0->train_nr() != 2292) {
        std::swap(e0, e1);
    }

    EXPECT_EQ(EventType_Arrival, e0->type());
    EXPECT_EQ(2292, e0->train_nr());
    EXPECT_EQ(std::string("381"), e0->line_id()->str());
    EXPECT_EQ(1448372400, e0->time());
    EXPECT_EQ(1448372400, e0->schedule_time());

    auto ie0 = e0->id_event();
    EXPECT_EQ(std::string("8000096"), ie0->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, ie0->type());
    EXPECT_EQ(2292, ie0->train_nr());
    EXPECT_EQ(std::string("381"), ie0->line_id()->str());
    EXPECT_EQ(1448366700, ie0->schedule_time());

    EXPECT_EQ(EventType_Arrival, e1->type());
    EXPECT_EQ(628, e1->train_nr());
    EXPECT_EQ(std::string(""), e1->line_id()->str());
    EXPECT_EQ(1448373900, e1->time());
    EXPECT_EQ(1448373840, e1->schedule_time());

    auto ie1 = e1->id_event();
    EXPECT_EQ(std::string("8000261"), ie1->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, ie1->type());
    EXPECT_EQ(628, ie1->train_nr());
    EXPECT_EQ(std::string(""), ie1->line_id()->str());
    EXPECT_EQ(1448362440, ie1->schedule_time());

    // only one departure
    auto e2 = resp->events()->Get(2);
    EXPECT_EQ(EventType_Departure, e2->type());
    EXPECT_EQ(628, e2->train_nr());
    EXPECT_EQ(std::string(""), e2->line_id()->str());
    EXPECT_EQ(1448374200, e2->time());
    EXPECT_EQ(1448374200, e2->schedule_time());

    auto ie2 = e2->id_event();
    EXPECT_EQ(std::string("8000261"), ie2->eva_nr()->str());
    EXPECT_EQ(EventType_Departure, ie2->type());
    EXPECT_EQ(628, ie2->train_nr());
    EXPECT_EQ(std::string(""), ie2->line_id()->str());
    EXPECT_EQ(1448362440, ie2->schedule_time());
  }
}

}  // namespace lookup
}  // namespace motis
