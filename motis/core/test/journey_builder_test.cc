#include "gtest/gtest.h"

#include "motis/core/common/journey.h"
#include "motis/core/common/journey_builder.h"

#include "motis/core/schedule/category.h"

#include "motis/routing/response_builder.h"

using namespace motis;
using namespace motis::journey_builder;

journey create_journey1() {
  journey j;
  j.duration = 30;
  j.price = 10;
  j.transfers = 1;

  j.stops.resize(4);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "1111111";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Stop0";
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445261400;
    stop.departure.platform = "1";
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "2222222";
    stop.index = 1;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Stop1";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445262000;
    stop.arrival.platform = "2";
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262240;
    stop.departure.platform = "3";
  }
  {
    auto& stop = j.stops[2];
    stop.eva_no = "3333333";
    stop.index = 2;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Stop2";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445262900;
    stop.arrival.platform = "4";
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
    stop.departure.platform = "";
  }
  {
    auto& stop = j.stops[3];
    stop.eva_no = "4444444";
    stop.index = 3;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Stop3";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263200;
    stop.arrival.platform = "";
    stop.departure.valid = false;
  }

  j.transports.resize(3);
  {
    auto& transport = j.transports[0];
    transport.category_id = 0;
    transport.category_name = "ICE";
    transport.direction = "X";
    transport.duration = 10;
    transport.from = 0;
    transport.line_identifier = "l1";
    transport.name = "ICE 111";
    transport.provider = "DB1";
    transport.slot = 0;
    transport.to = 1;
    transport.train_nr = 111;
    transport.walk = false;
  }
  {
    auto& transport = j.transports[1];
    transport.category_id = 1;
    transport.category_name = "IC";
    transport.direction = "Y";
    transport.duration = 11;
    transport.from = 1;
    transport.line_identifier = "l2";
    transport.name = "IC 222";
    transport.provider = "DB2";
    transport.slot = 0;
    transport.to = 2;
    transport.train_nr = 222;
    transport.walk = false;
  }
  {
    auto& transport = j.transports[2];
    transport.walk = true;
    transport.duration = 5;
    transport.from = 2;
    transport.to = 3;
    transport.category_id = 0;
    transport.category_name = "";
    transport.direction = "";
    transport.line_identifier = "";
    transport.name = "";
    transport.provider = "";
    transport.slot = 0;
    transport.train_nr = 0;
  }

  j.attributes.resize(2);
  {
    auto& attribute = j.attributes[0];
    attribute.code = "A";
    attribute.from = 0;
    attribute.to = 1;
    attribute.text = "AAA";
  }
  {
    auto& attribute = j.attributes[1];
    attribute.code = "B";
    attribute.from = 1;
    attribute.to = 2;
    attribute.text = "BBB";
  }
  return j;
}

journey create_journey2() {
  journey j;
  j.duration = 15;
  j.price = 10;
  j.transfers = 0;

  j.stops.resize(2);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "1111111";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Stop0";
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445328000;
    stop.departure.platform = "1";
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "2222222";
    stop.index = 1;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Stop1";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445328900;
    stop.arrival.platform = "2";
    stop.departure.valid = false;
    stop.departure.timestamp = 0;
    stop.departure.platform = "3";
  }
  j.transports.resize(1);
  {
    auto& transport = j.transports[0];
    transport.category_id = 0;
    transport.category_name = "ICE";
    transport.direction = "X";
    transport.duration = 15;
    transport.from = 0;
    transport.line_identifier = "l1";
    transport.name = "ICE 111";
    transport.provider = "DB1";
    transport.slot = 0;
    transport.to = 1;
    transport.train_nr = 111;
    transport.walk = false;
  }
  return j;
}

TEST(core_convert_journey, journey_builder) {
  std::vector<journey> original_journeys;
  original_journeys.push_back(create_journey1());
  original_journeys.push_back(create_journey2());

  auto msg = routing::journeys_to_message(original_journeys);
  auto journeys = to_journeys(msg->content<routing::RoutingResponse const*>());

  ASSERT_TRUE(journeys.size() == 2);

  for (unsigned int i = 0; i < 2; ++i) {
    auto const& j = journeys[i];
    auto const& o = original_journeys[i];

    ASSERT_TRUE(o.duration == j.duration);
    // ASSERT_TRUE(o.price == j.price);
    ASSERT_TRUE(o.transfers == j.transfers);
    ASSERT_TRUE(o.stops.size() == j.stops.size());
    ASSERT_TRUE(o.transports.size() == j.transports.size());
    ASSERT_TRUE(o.attributes.size() == j.attributes.size());

    for (unsigned int s = 0; s < o.stops.size(); ++s) {
      auto const& os = o.stops[s];
      auto const& js = j.stops[s];
      ASSERT_TRUE(os.eva_no == js.eva_no);
      ASSERT_TRUE(os.index == js.index);
      ASSERT_TRUE(os.interchange == js.interchange);
      ASSERT_TRUE(os.lat == js.lat);
      ASSERT_TRUE(os.lng == js.lng);
      ASSERT_TRUE(os.name == js.name);
      ASSERT_TRUE(os.arrival.platform == js.arrival.platform);
      ASSERT_TRUE(os.arrival.timestamp == js.arrival.timestamp);
      ASSERT_TRUE(os.arrival.valid == js.arrival.valid);
      ASSERT_TRUE(os.departure.platform == js.departure.platform);
      ASSERT_TRUE(os.departure.timestamp == js.departure.timestamp);
      ASSERT_TRUE(os.departure.valid == js.departure.valid);
    }

    for (unsigned int t = 0; t < o.transports.size(); ++t) {
      auto const& ot = o.transports[t];
      auto const& jt = j.transports[t];
      ASSERT_TRUE(ot.category_id == jt.category_id);
      ASSERT_TRUE(ot.category_name == jt.category_name);
      ASSERT_TRUE(ot.direction == jt.direction);
      ASSERT_TRUE(ot.duration == jt.duration);
      ASSERT_TRUE(ot.from == jt.from);
      ASSERT_TRUE(ot.line_identifier == jt.line_identifier);
      ASSERT_TRUE(ot.name == jt.name);
      ASSERT_TRUE(ot.provider == jt.provider);
      ASSERT_TRUE(ot.slot == jt.slot);
      ASSERT_TRUE(ot.to == jt.to);
      ASSERT_TRUE(ot.train_nr == jt.train_nr);
      ASSERT_TRUE(ot.walk == jt.walk);
    }

    for (unsigned int a = 0; a < o.attributes.size(); ++a) {
      auto const& oa = o.attributes[a];
      auto const& ja = j.attributes[a];
      ASSERT_TRUE(oa.code == ja.code);
      ASSERT_TRUE(oa.from == ja.from);
      ASSERT_TRUE(oa.to == ja.to);
      ASSERT_TRUE(oa.text == ja.text);
    }
  }
}
