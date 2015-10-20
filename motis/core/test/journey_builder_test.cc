#include "catch/catch.hpp"

#include "motis/core/common/journey.h"
#include "motis/core/common/journey_builder.h"

#include "motis/core/schedule/category.h"

#include "motis/routing/response_builder.h"

using namespace motis;

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

#include <iostream>

TEST_CASE("convert journey", "[journey_builder]") {
  std::vector<journey> original_journeys;
  original_journeys.push_back(create_journey1());
  original_journeys.push_back(create_journey2());
  std::vector<std::unique_ptr<category>> categories;
  categories.emplace_back(new category("ICE", 0));
  categories.emplace_back(new category("IC", 0));

  auto msg = routing::journeys_to_message(original_journeys);
  auto journeys =
      to_journeys(msg->content<routing::RoutingResponse const*>(), categories);

  REQUIRE(journeys.size() == 2);

  for (unsigned int i = 0; i < 2; ++i) {
    auto const& j = journeys[i];
    auto const& o = original_journeys[i];

    REQUIRE(o.duration == j.duration);
    // REQUIRE(o.price == j.price);
    REQUIRE(o.transfers == j.transfers);
    REQUIRE(o.stops.size() == j.stops.size());
    REQUIRE(o.transports.size() == j.transports.size());
    REQUIRE(o.attributes.size() == j.attributes.size());

    for (unsigned int s = 0; s < o.stops.size(); ++s) {
      auto const& os = o.stops[s];
      auto const& js = j.stops[s];
      REQUIRE(os.eva_no == js.eva_no);
      REQUIRE(os.index == js.index);
      REQUIRE(os.interchange == js.interchange);
      REQUIRE(os.lat == js.lat);
      REQUIRE(os.lng == js.lng);
      REQUIRE(os.name == js.name);
      REQUIRE(os.arrival.platform == js.arrival.platform);
      REQUIRE(os.arrival.timestamp == js.arrival.timestamp);
      REQUIRE(os.arrival.valid == js.arrival.valid);
      REQUIRE(os.departure.platform == js.departure.platform);
      REQUIRE(os.departure.timestamp == js.departure.timestamp);
      REQUIRE(os.departure.valid == js.departure.valid);
    }

    for (unsigned int t = 0; t < o.transports.size(); ++t) {
      auto const& ot = o.transports[t];
      auto const& jt = j.transports[t];
      REQUIRE(ot.category_id == jt.category_id);
      REQUIRE(ot.category_name == jt.category_name);
      REQUIRE(ot.direction == jt.direction);
      REQUIRE(ot.duration == jt.duration);
      REQUIRE(ot.from == jt.from);
      REQUIRE(ot.line_identifier == jt.line_identifier);
      REQUIRE(ot.name == jt.name);
      REQUIRE(ot.provider == jt.provider);
      REQUIRE(ot.slot == jt.slot);
      REQUIRE(ot.to == jt.to);
      REQUIRE(ot.train_nr == jt.train_nr);
      REQUIRE(ot.walk == jt.walk);
    }

    for (unsigned int a = 0; a < o.attributes.size(); ++a) {
      auto const& oa = o.attributes[a];
      auto const& ja = j.attributes[a];
      REQUIRE(oa.code == ja.code);
      REQUIRE(oa.from == ja.from);
      REQUIRE(oa.to == ja.to);
      REQUIRE(oa.text == ja.text);
    }
  }
}
