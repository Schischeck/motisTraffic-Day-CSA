#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule::simple_realtime;

namespace motis {
namespace lookup {

constexpr auto kNotInPeriod = R""(
{ "destination": {"type": "Module", "target": "/lookup/station_events"},
  "content_type": "LookupStationEventsRequest",
  "content": { "eva_nr": "foo", "begin": 0, "end": 0 }}
)"";

constexpr auto kSiegenEmptyRequest = R""(
{ "destination": {"type": "Module", "target": "/lookup/station_events"},
  "content_type": "LookupStationEventsRequest",
  "content": {
    "eva_nr": "8000046",  // Siegen Hbf
    "begin": 1448373600,  // 2015-11-24 15:00:00 GMT+0100
    "end": 1448374200  // 2015-11-24 15:10:00 GMT+0100
  }}
)"";

constexpr auto kSiegenRequest = R""(
{ "destination": {"type": "Module", "target": "/lookup/station_events"},
  "content_type": "LookupStationEventsRequest",
  "content": {
    "eva_nr": "8000046",  // Siegen Hbf
    "begin": 1448373600,  // 2015-11-24 15:00:00 GMT+0100
    "end": 1448374260  // 2015-11-24 15:11:00 GMT+0100
  }}
)"";

constexpr auto kFrankfurtRequest = R""(
{ "destination": {"type": "Module", "target": "/lookup/station_events"},
  "content_type": "LookupStationEventsRequest",
  "content": {
    "eva_nr": "8000105",  // Frankfurt(Main)Hbf
    "begin": 1448371800,  // 2015-11-24 14:30:00 GMT+0100
    "end": 1448375400  // 2015-11-24 15:30:00 GMT+0100
  }}
)"";

// TODO(sebastian) re-enable when working realtime module is available
TEST(lookup, DISABLED_station_events) {
  auto motis =
      launch_motis(kSchedulePath, kScheduleDate, {"lookup", "realtime"});
  call(motis, get_ris_message());

  ASSERT_ANY_THROW(call(motis, make_msg(kNotInPeriod)));

  {
    auto msg = call(motis, make_msg(kSiegenEmptyRequest));
    auto resp = motis_content(LookupStationEventsResponse, msg);
    ASSERT_EQ(0, resp->events()->size());  // end is exclusive
  }
  {
    auto msg = call(motis, make_msg(kSiegenRequest));
    auto resp = motis_content(LookupStationEventsResponse, msg);
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
    auto msg = call(motis, make_msg(kFrankfurtRequest));
    auto resp = motis_content(LookupStationEventsResponse, msg);

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

TEST(lookup, station_events_no_realtime) {
  auto instance = launch_motis(kSchedulePath, kScheduleDate, {"lookup"});

  ASSERT_ANY_THROW(call(instance, make_msg(kNotInPeriod)));

  {
    auto msg = call(instance, make_msg(kSiegenEmptyRequest));
    auto resp = motis_content(LookupStationEventsResponse, msg);
    ASSERT_EQ(0, resp->events()->size());  // end is exclusive
  }
  {
    auto msg = call(instance, make_msg(kSiegenRequest));
    auto resp = motis_content(LookupStationEventsResponse, msg);
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
    auto msg = call(instance, make_msg(kFrankfurtRequest));
    auto resp = motis_content(LookupStationEventsResponse, msg);

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
          EXPECT_EQ(1448373840, e->time());
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
