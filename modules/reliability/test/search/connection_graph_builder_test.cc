#include "gtest/gtest.h"

#include "motis/core/journey/journey.h"

#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_builder {

journey create_journey1() {
  journey j;
  j.duration = 21;
  j.price = 10;
  j.transfers = 1;

  j.stops.resize(5);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "0000000";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station0";
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445261400;
    stop.departure.platform = "1";
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "1111111";
    stop.index = 1;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station1";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445262000;
    stop.arrival.platform = "2";
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262240;
    stop.departure.platform = "3";
  }
  {
    auto& stop = j.stops[2];
    stop.eva_no = "2222222";
    stop.index = 2;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station2";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445262900;
    stop.arrival.platform = "4";
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
    stop.departure.platform = "";
  }
  {
    auto& stop = j.stops[3];
    stop.eva_no = "3333333";
    stop.index = 3;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station3";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263200;
    stop.arrival.platform = "";
    stop.departure.valid = true;
    stop.departure.timestamp = 1445263320;
    stop.departure.platform = "5";
  }
  {
    auto& stop = j.stops[4];
    stop.eva_no = "4444444";
    stop.index = 4;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station4";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263920;
    stop.arrival.platform = "6";
    stop.departure.valid = false;
  }

  j.transports.resize(4);
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
    transport.route_id = 1;
    transport.slot = 0;
    transport.to = 1;
    transport.train_nr = 111;
    transport.type = journey::transport::PublicTransport;
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
    transport.route_id = 2;
    transport.slot = 0;
    transport.to = 2;
    transport.train_nr = 222;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[2];
    transport.type = journey::transport::Walk;
    transport.duration = 5;
    transport.from = 2;
    transport.to = 3;
    transport.category_id = 0;
    transport.category_name = "";
    transport.direction = "";
    transport.line_identifier = "";
    transport.name = "";
    transport.provider = "";
    transport.route_id = 0;
    transport.slot = 0;
    transport.train_nr = 0;
  }
  {
    auto& transport = j.transports[3];
    transport.category_id = 3;
    transport.category_name = "RB";
    transport.direction = "Z";
    transport.duration = 10;
    transport.from = 3;
    transport.line_identifier = "l3";
    transport.name = "RB 333";
    transport.provider = "DB3";
    transport.route_id = 3;
    transport.slot = 0;
    transport.to = 4;
    transport.train_nr = 333;
    transport.type = journey::transport::PublicTransport;
  }

  j.attributes.resize(4);
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
  {
    auto& attribute = j.attributes[2];
    attribute.code = "C";
    attribute.from = 0;
    attribute.to = 2;
    attribute.text = "CCC";
  }
  {
    auto& attribute = j.attributes[3];
    attribute.code = "D";
    attribute.from = 3;
    attribute.to = 4;
    attribute.text = "DDD";
  }
  return j;
}

TEST(reliability_connection_graph_builder, split_journey) {
  auto const journeys = detail::split_journey(create_journey1());
  ASSERT_EQ(journeys.size(), 3);
  {
    auto const& journey = journeys[0];
    ASSERT_EQ(journey.duration, 10);
    ASSERT_EQ(journey.price, 0);
    ASSERT_EQ(journey.transfers, 0);
    ASSERT_EQ(journey.stops.size(), 2);
    {
      auto& stop = journey.stops[0];
      ASSERT_EQ(stop.eva_no, "0000000");
      ASSERT_EQ(stop.index, 0);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station0");
      ASSERT_FALSE(stop.arrival.valid);
      ASSERT_TRUE(stop.departure.valid);
      ASSERT_EQ(stop.departure.timestamp, 1445261400);
      ASSERT_EQ(stop.departure.platform, "1");
    }
    {
      auto& stop = journey.stops[1];
      ASSERT_EQ(stop.eva_no, "1111111");
      ASSERT_EQ(stop.index, 1);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station1");
      ASSERT_TRUE(stop.arrival.valid);
      ASSERT_EQ(stop.arrival.timestamp, 1445262000);
      ASSERT_EQ(stop.arrival.platform, "2");
      ASSERT_FALSE(stop.departure.valid);
    }
    ASSERT_EQ(journey.transports.size(), 1);
    {
      auto& transport = journey.transports[0];
      ASSERT_EQ(transport.category_id, 0);
      ASSERT_EQ(transport.category_name, "ICE");
      ASSERT_EQ(transport.direction, "X");
      ASSERT_EQ(transport.duration, 10);
      ASSERT_EQ(transport.from, 0);
      ASSERT_EQ(transport.line_identifier, "l1");
      ASSERT_EQ(transport.name, "ICE 111");
      ASSERT_EQ(transport.provider, "DB1");
      ASSERT_EQ(1, transport.route_id);
      ASSERT_EQ(transport.slot, 0);
      ASSERT_EQ(transport.to, 1);
      ASSERT_EQ(transport.train_nr, 111);
      ASSERT_EQ(journey::transport::PublicTransport, transport.type);
    }
    ASSERT_EQ(journey.attributes.size(), 2);
    {
      auto& attribute = journey.attributes[0];
      ASSERT_EQ(attribute.code, "A");
      ASSERT_EQ(attribute.from, 0);
      ASSERT_EQ(attribute.to, 1);
      ASSERT_EQ(attribute.text, "AAA");
    }
    {
      auto& attribute = journey.attributes[1];
      ASSERT_EQ(attribute.code, "C");
      ASSERT_EQ(attribute.from, 0);
      ASSERT_EQ(attribute.to, 1);
      ASSERT_EQ(attribute.text, "CCC");
    }
  }
  {
    auto const& journey = journeys[1];
    ASSERT_EQ(journey.duration, 11);
    ASSERT_EQ(journey.price, 0);
    ASSERT_EQ(journey.transfers, 0);
    ASSERT_EQ(journey.stops.size(), 2);
    {
      auto& stop = journey.stops[0];
      ASSERT_EQ(stop.eva_no, "1111111");
      ASSERT_EQ(stop.index, 0);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station1");
      ASSERT_FALSE(stop.arrival.valid);
      ASSERT_TRUE(stop.departure.valid);
      ASSERT_EQ(stop.departure.timestamp, 1445262240);
      ASSERT_EQ(stop.departure.platform, "3");
    }
    {
      auto& stop = journey.stops[1];
      ASSERT_EQ(stop.eva_no, "2222222");
      ASSERT_EQ(stop.index, 1);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station2");
      ASSERT_TRUE(stop.arrival.valid);
      ASSERT_EQ(stop.arrival.timestamp, 1445262900);
      ASSERT_EQ(stop.arrival.platform, "4");
      ASSERT_FALSE(stop.departure.valid);
    }
    ASSERT_EQ(journey.transports.size(), 1);
    {
      auto& transport = journey.transports[0];
      ASSERT_EQ(transport.category_id, 1);
      ASSERT_EQ(transport.category_name, "IC");
      ASSERT_EQ(transport.direction, "Y");
      ASSERT_EQ(transport.duration, 11);
      ASSERT_EQ(transport.from, 0);
      ASSERT_EQ(transport.line_identifier, "l2");
      ASSERT_EQ(transport.name, "IC 222");
      ASSERT_EQ(transport.provider, "DB2");
      ASSERT_EQ(2, transport.route_id);
      ASSERT_EQ(transport.slot, 0);
      ASSERT_EQ(transport.to, 1);
      ASSERT_EQ(transport.train_nr, 222);
      ASSERT_EQ(journey::transport::PublicTransport, transport.type);
    }
    ASSERT_EQ(journey.attributes.size(), 2);
    {
      auto& attribute = journey.attributes[0];
      ASSERT_EQ(attribute.code, "B");
      ASSERT_EQ(attribute.from, 0);
      ASSERT_EQ(attribute.to, 1);
      ASSERT_EQ(attribute.text, "BBB");
    }
    {
      auto& attribute = journey.attributes[1];
      ASSERT_EQ(attribute.code, "C");
      ASSERT_EQ(attribute.from, 0);
      ASSERT_EQ(attribute.to, 1);
      ASSERT_EQ(attribute.text, "CCC");
    }
  }
  {
    auto const& journey = journeys[2];
    ASSERT_EQ(journey.duration, 17);
    ASSERT_EQ(journey.price, 0);
    ASSERT_EQ(journey.transfers, 0);
    ASSERT_EQ(journey.stops.size(), 3);
    {
      auto& stop = journey.stops[0];
      ASSERT_EQ(stop.eva_no, "2222222");
      ASSERT_EQ(stop.index, 0);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station2");
      ASSERT_FALSE(stop.arrival.valid);
      ASSERT_TRUE(stop.departure.valid);
      ASSERT_EQ(stop.departure.timestamp, 1445262900);
      ASSERT_EQ(stop.departure.platform, "");
    }
    {
      auto& stop = journey.stops[1];
      ASSERT_EQ(stop.eva_no, "3333333");
      ASSERT_EQ(stop.index, 1);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station3");
      ASSERT_TRUE(stop.arrival.valid);
      ASSERT_EQ(stop.arrival.timestamp, 1445263200);
      ASSERT_EQ(stop.arrival.platform, "");
      ASSERT_TRUE(stop.departure.valid);
      ASSERT_EQ(stop.departure.timestamp, 1445263320);
      ASSERT_EQ(stop.departure.platform, "5");
    }
    {
      auto& stop = journey.stops[2];
      ASSERT_EQ(stop.eva_no, "4444444");
      ASSERT_EQ(stop.index, 2);
      ASSERT_FALSE(stop.interchange);
      ASSERT_EQ(stop.lat, 0.0);
      ASSERT_EQ(stop.lng, 0.0);
      ASSERT_EQ(stop.name, "Station4");
      ASSERT_TRUE(stop.arrival.valid);
      ASSERT_EQ(stop.arrival.timestamp, 1445263920);
      ASSERT_EQ(stop.arrival.platform, "6");
      ASSERT_FALSE(stop.departure.valid);
    }
    ASSERT_EQ(journey.transports.size(), 2);
    {
      auto& transport = journey.transports[0];
      ASSERT_EQ(journey::transport::Walk, transport.type);
      ASSERT_EQ(transport.duration, 5);
      ASSERT_EQ(transport.from, 0);
      ASSERT_EQ(transport.to, 1);
      ASSERT_EQ(transport.category_id, 0);
      ASSERT_EQ(transport.category_name, "");
      ASSERT_EQ(transport.direction, "");
      ASSERT_EQ(transport.line_identifier, "");
      ASSERT_EQ(transport.name, "");
      ASSERT_EQ(transport.provider, "");
      ASSERT_EQ(transport.slot, 0);
      ASSERT_EQ(transport.train_nr, 0);
    }
    {
      auto& transport = journey.transports[1];
      ASSERT_EQ(transport.category_id, 3);
      ASSERT_EQ(transport.category_name, "RB");
      ASSERT_EQ(transport.direction, "Z");
      ASSERT_EQ(transport.duration, 10);
      ASSERT_EQ(transport.from, 1);
      ASSERT_EQ(transport.line_identifier, "l3");
      ASSERT_EQ(transport.name, "RB 333");
      ASSERT_EQ(transport.provider, "DB3");
      ASSERT_EQ(3, transport.route_id);
      ASSERT_EQ(transport.slot, 0);
      ASSERT_EQ(transport.to, 2);
      ASSERT_EQ(transport.train_nr, 333);
      ASSERT_EQ(journey::transport::PublicTransport, transport.type);
    }
    ASSERT_EQ(journey.attributes.size(), 1);
    {
      auto& attribute = journey.attributes[0];
      ASSERT_EQ(attribute.code, "D");
      ASSERT_EQ(attribute.from, 1);
      ASSERT_EQ(attribute.to, 2);
      ASSERT_EQ(attribute.text, "DDD");
    }
  }
}

journey create_journey2() {
  journey j;
  j.duration = 20;
  j.price = 0;
  j.transfers = 0;

  j.stops.resize(2);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "0000000";
    stop.name = "Station0";
    stop.index = 0;
    stop.interchange = false;
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445261400;
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "1111111";
    stop.name = "Station1";
    stop.index = 1;
    stop.interchange = false;
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445262600;
    stop.departure.valid = false;
  }

  j.transports.resize(1);
  {
    auto& transport = j.transports[0];
    transport.duration = 10;
    transport.from = 0;
    transport.to = 1;
    transport.train_nr = 111;
    transport.type = journey::transport::PublicTransport;
  }
  return j;
}

TEST(reliability_connection_graph_builder, add_base_journey1) {
  connection_graph cg;
  add_base_journey(cg, create_journey1());

  ASSERT_EQ(4, cg.stops_.size());
  {
    auto const& stop = cg.stops_[0];
    ASSERT_EQ(stop.index_, 0);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 0);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 2);
    ASSERT_EQ(cg.station_info(0).first, "Station0");
    ASSERT_EQ(cg.station_info(0).second, "0000000");
  }
  {
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(stop.index_, 1);
    ASSERT_EQ(stop.alternative_infos_.size(), 0);
    ASSERT_EQ(cg.station_info(1).first, "Station4");
    ASSERT_EQ(cg.station_info(1).second, "4444444");
  }
  {
    auto const& stop = cg.stops_[2];
    ASSERT_EQ(stop.index_, 2);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 1);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 3);
    ASSERT_EQ(cg.station_info(2).first, "Station1");
    ASSERT_EQ(cg.station_info(2).second, "1111111");
  }
  {
    auto const& stop = cg.stops_[3];
    ASSERT_EQ(stop.index_, 3);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 2);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(3).first, "Station2");
    ASSERT_EQ(cg.station_info(3).second, "2222222");
  }

  ASSERT_EQ(cg.journeys_.size(), 3);
  {
    auto const& j = cg.journeys_[0];
    ASSERT_EQ(j.stops.front().eva_no, "0000000");
    ASSERT_EQ(j.stops.back().eva_no, "1111111");
    ASSERT_EQ(j.transports.front().train_nr, 111);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "2222222");
    ASSERT_EQ(j.transports.front().train_nr, 222);
  }
  {
    auto const& j = cg.journeys_[2];
    ASSERT_EQ(j.stops.front().eva_no, "2222222");
    ASSERT_EQ(j.stops.back().eva_no, "4444444");
    ASSERT_EQ(j.transports.back().train_nr, 333);
  }
}

TEST(reliability_connection_graph_builder, add_base_journey2) {
  connection_graph cg;
  add_base_journey(cg, create_journey2());

  ASSERT_EQ(cg.stops_.size(), 2);
  {
    auto const& stop = cg.stops_[0];
    ASSERT_EQ(stop.index_, 0);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 0);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(0).first, "Station0");
    ASSERT_EQ(cg.station_info(0).second, "0000000");
  }
  {
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(stop.index_, 1);
    ASSERT_EQ(stop.alternative_infos_.size(), 0);
    ASSERT_EQ(cg.station_info(1).first, "Station1");
    ASSERT_EQ(cg.station_info(1).second, "1111111");
  }

  ASSERT_EQ(cg.journeys_.size(), 1);
  {
    auto const& j = cg.journeys_[0];
    ASSERT_EQ(j.stops.front().eva_no, "0000000");
    ASSERT_EQ(j.stops.back().eva_no, "1111111");
    ASSERT_EQ(j.transports.front().train_nr, 111);
  }
}

journey create_journey3() {
  journey j;
  j.transfers = 0;

  j.stops.resize(2);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "1111111";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station1";
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262600;
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "4444444";
    stop.index = 1;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station4";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445264400;
    stop.departure.valid = false;
  }

  j.transports.resize(1);
  {
    auto& transport = j.transports[0];
    transport.from = 0;
    transport.to = 1;
    transport.train_nr = 555;
    transport.type = journey::transport::PublicTransport;
  }
  return j;
}

TEST(reliability_connection_graph_builder, add_alternative_journey) {
  connection_graph cg;
  add_base_journey(cg, create_journey1());
  add_alternative_journey(cg, 2, create_journey3());

  ASSERT_EQ(4, cg.stops_.size());
  {
    auto const& stop = cg.stops_[0];
    ASSERT_EQ(stop.index_, 0);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 0);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 2);
    ASSERT_EQ(cg.station_info(0).first, "Station0");
    ASSERT_EQ(cg.station_info(0).second, "0000000");
  }
  {
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(stop.index_, 1);
    ASSERT_EQ(stop.alternative_infos_.size(), 0);
    ASSERT_EQ("Station4", cg.station_info(1).first);
    ASSERT_EQ("4444444", cg.station_info(1).second);
  }
  {
    auto const& stop = cg.stops_[2];
    ASSERT_EQ(stop.index_, 2);
    ASSERT_EQ(stop.alternative_infos_.size(), 2);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 1);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 3);
    ASSERT_EQ(stop.alternative_infos_[1].journey_index_, 3);
    ASSERT_EQ(stop.alternative_infos_[1].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(2).first, "Station1");
    ASSERT_EQ(cg.station_info(2).second, "1111111");
  }
  {
    auto const& stop = cg.stops_[3];
    ASSERT_EQ(stop.index_, 3);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 2);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(3).first, "Station2");
    ASSERT_EQ(cg.station_info(3).second, "2222222");
  }

  ASSERT_EQ(cg.journeys_.size(), 4);
  {
    auto const& j = cg.journeys_[0];
    ASSERT_EQ(j.stops.front().eva_no, "0000000");
    ASSERT_EQ(j.stops.back().eva_no, "1111111");
    ASSERT_EQ(j.transports.front().train_nr, 111);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "2222222");
    ASSERT_EQ(j.transports.front().train_nr, 222);
  }
  {
    auto const& j = cg.journeys_[2];
    ASSERT_EQ(j.stops.front().eva_no, "2222222");
    ASSERT_EQ(j.stops.back().eva_no, "4444444");
    ASSERT_EQ(j.transports.back().train_nr, 333);
  }
  {
    auto const& j = cg.journeys_[3];
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "4444444");
    ASSERT_EQ(j.transports.back().train_nr, 555);
  }
}

journey create_journey4() {
  journey j;
  j.transfers = 1;

  j.stops.resize(3);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "1111111";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station1";
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "5555555";
    stop.index = 1;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station5";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263200;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445263500;
  }
  {
    auto& stop = j.stops[2];
    stop.eva_no = "4444444";
    stop.index = 2;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station4";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263800;
    stop.departure.valid = false;
  }

  j.transports.resize(2);
  {
    auto& transport = j.transports[0];
    transport.from = 0;
    transport.to = 1;
    transport.train_nr = 666;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[1];
    transport.from = 1;
    transport.to = 2;
    transport.train_nr = 777;
    transport.type = journey::transport::PublicTransport;
  }
  return j;
}

TEST(reliability_connection_graph_builder, add_alternative_journey2) {
  connection_graph cg;
  add_base_journey(cg, create_journey1());
  add_alternative_journey(cg, 2, create_journey3());
  add_alternative_journey(cg, 2, create_journey4());

  ASSERT_EQ(5, cg.stops_.size());
  {
    auto const& stop = cg.stops_[0];
    ASSERT_EQ(stop.index_, 0);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 0);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 2);
    ASSERT_EQ(cg.station_info(0).first, "Station0");
    ASSERT_EQ(cg.station_info(0).second, "0000000");
  }
  {
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(stop.index_, 1);
    ASSERT_EQ(stop.alternative_infos_.size(), 0);
    ASSERT_EQ("Station4", cg.station_info(1).first);
    ASSERT_EQ("4444444", cg.station_info(1).second);
  }
  {
    auto const& stop = cg.stops_[2];
    ASSERT_EQ(stop.index_, 2);
    ASSERT_EQ(stop.alternative_infos_.size(), 3);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 1);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 3);
    ASSERT_EQ(stop.alternative_infos_[1].journey_index_, 3);
    ASSERT_EQ(stop.alternative_infos_[1].next_stop_index_, 1);
    ASSERT_EQ(stop.alternative_infos_[2].journey_index_, 4);
    ASSERT_EQ(stop.alternative_infos_[2].next_stop_index_, 4);
    ASSERT_EQ(cg.station_info(2).first, "Station1");
    ASSERT_EQ(cg.station_info(2).second, "1111111");
  }
  {
    auto const& stop = cg.stops_[3];
    ASSERT_EQ(stop.index_, 3);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 2);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(3).first, "Station2");
    ASSERT_EQ(cg.station_info(3).second, "2222222");
  }
  {
    auto const& stop = cg.stops_[4];
    ASSERT_EQ(stop.index_, 4);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 5);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(4).first, "Station5");
    ASSERT_EQ(cg.station_info(4).second, "5555555");
  }

  ASSERT_EQ(cg.journeys_.size(), 6);
  {
    auto const& j = cg.journeys_[0];
    ASSERT_EQ(j.stops.front().eva_no, "0000000");
    ASSERT_EQ(j.stops.back().eva_no, "1111111");
    ASSERT_EQ(j.transports.front().train_nr, 111);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "2222222");
    ASSERT_EQ(j.transports.front().train_nr, 222);
  }
  {
    auto const& j = cg.journeys_[2];
    ASSERT_EQ(j.stops.front().eva_no, "2222222");
    ASSERT_EQ(j.stops.back().eva_no, "4444444");
    ASSERT_EQ(j.transports.back().train_nr, 333);
  }
  {
    auto const& j = cg.journeys_[3];
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "4444444");
    ASSERT_EQ(j.transports.back().train_nr, 555);
  }
  {
    auto const& j = cg.journeys_[4];
    ASSERT_EQ("1111111", j.stops.front().eva_no);
    ASSERT_EQ("5555555", j.stops.back().eva_no);
    ASSERT_EQ(666, j.transports.back().train_nr);
  }
  {
    auto const& j = cg.journeys_[5];
    ASSERT_EQ(j.stops.front().eva_no, "5555555");
    ASSERT_EQ(j.stops.back().eva_no, "4444444");
    ASSERT_EQ(j.transports.back().train_nr, 777);
  }
}

journey create_journey5() {
  journey j;
  j.transfers = 1;

  j.stops.resize(5);
  {
    auto& stop = j.stops[0];
    stop.name = "DUMMY";
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "1111111";
    stop.index = 1;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station1";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445262900;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
  }
  {
    auto& stop = j.stops[2];
    stop.eva_no = "2222222";
    stop.index = 2;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station2";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263200;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445263500;
  }
  {
    auto& stop = j.stops[3];
    stop.eva_no = "3333333";
    stop.index = 3;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station3";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263800;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445263800;
  }
  {
    auto& stop = j.stops[4];
    stop.index = 4;
    stop.interchange = false;
    stop.name = "DUMMY";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263800;
    stop.departure.valid = false;
  }

  j.transports.resize(4);
  {
    auto& transport = j.transports[0];
    transport.from = 0;
    transport.to = 1;
    transport.type = journey::transport::Walk;
  }
  {
    auto& transport = j.transports[1];
    transport.from = 1;
    transport.to = 2;
    transport.train_nr = 666;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[2];
    transport.from = 2;
    transport.to = 3;
    transport.train_nr = 777;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[3];
    transport.from = 3;
    transport.to = 4;
    transport.type = journey::transport::Walk;
  }

  j.attributes.resize(3);
  {
    auto& attribute = j.attributes[0];
    attribute.code = "A";
    attribute.from = 1;
    attribute.to = 3;
  }
  {
    auto& attribute = j.attributes[1];
    attribute.code = "B";
    attribute.from = 1;
    attribute.to = 2;
  }
  {
    auto& attribute = j.attributes[2];
    attribute.code = "C";
    attribute.from = 2;
    attribute.to = 3;
  }
  return j;
}

TEST(reliability_connection_graph_builder, remove_dummy_stops) {
  journey const j = detail::remove_dummy_stops(create_journey5());
  ASSERT_EQ(j.transfers, 1);

  ASSERT_EQ(3, j.stops.size());
  {
    auto& stop = j.stops[0];
    ASSERT_EQ(stop.eva_no, "1111111");
    ASSERT_EQ(stop.index, 0);
    ASSERT_EQ(stop.interchange, false);
    ASSERT_EQ(stop.lat, 0.0);
    ASSERT_EQ(stop.lng, 0.0);
    ASSERT_EQ(stop.name, "Station1");
    ASSERT_EQ(stop.arrival.valid, false);
    ASSERT_EQ(stop.departure.valid, true);
    ASSERT_EQ(stop.departure.timestamp, 1445262900);
  }
  {
    auto& stop = j.stops[1];
    ASSERT_EQ(stop.eva_no, "2222222");
    ASSERT_EQ(stop.index, 1);
    ASSERT_EQ(stop.interchange, true);
    ASSERT_EQ(stop.lat, 0.0);
    ASSERT_EQ(stop.lng, 0.0);
    ASSERT_EQ(stop.name, "Station2");
    ASSERT_EQ(stop.arrival.valid, true);
    ASSERT_EQ(stop.arrival.timestamp, 1445263200);
    ASSERT_EQ(stop.departure.valid, true);
    ASSERT_EQ(stop.departure.timestamp, 1445263500);
  }
  {
    auto& stop = j.stops[2];
    ASSERT_EQ(stop.eva_no, "3333333");
    ASSERT_EQ(stop.index, 2);
    ASSERT_EQ(stop.interchange, false);
    ASSERT_EQ(stop.lat, 0.0);
    ASSERT_EQ(stop.lng, 0.0);
    ASSERT_EQ(stop.name, "Station3");
    ASSERT_EQ(stop.arrival.valid, true);
    ASSERT_EQ(stop.arrival.timestamp, 1445263800);
    ASSERT_EQ(stop.departure.valid, false);
  }

  ASSERT_EQ(2, j.transports.size());
  {
    auto& transport = j.transports[0];
    ASSERT_EQ(transport.from, 0);
    ASSERT_EQ(transport.to, 1);
    ASSERT_EQ(transport.train_nr, 666);
    ASSERT_EQ(journey::transport::PublicTransport, transport.type);
  }
  {
    auto& transport = j.transports[1];
    ASSERT_EQ(transport.from, 1);
    ASSERT_EQ(transport.to, 2);
    ASSERT_EQ(transport.train_nr, 777);
    ASSERT_EQ(journey::transport::PublicTransport, transport.type);
  }

  ASSERT_EQ(3, j.attributes.size());
  {
    auto& attribute = j.attributes[0];
    ASSERT_EQ(attribute.code, "A");
    ASSERT_EQ(attribute.from, 0);
    ASSERT_EQ(attribute.to, 2);
  }
  {
    auto& attribute = j.attributes[1];
    ASSERT_EQ(attribute.code, "B");
    ASSERT_EQ(attribute.from, 0);
    ASSERT_EQ(attribute.to, 1);
  }
  {
    auto& attribute = j.attributes[2];
    ASSERT_EQ(attribute.code, "C");
    ASSERT_EQ(attribute.from, 1);
    ASSERT_EQ(attribute.to, 2);
  }
}

journey create_journey_train_id_change() {
  journey j;
  j.transfers = 1;

  j.stops.resize(3);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "1111111";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station1";
    stop.arrival.valid = false;
    stop.arrival.timestamp = 1445262900;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "2222222";
    stop.index = 1;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station2";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263200;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445263500;
  }
  {
    auto& stop = j.stops[2];
    stop.eva_no = "3333333";
    stop.index = 2;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station3";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263800;
    stop.departure.valid = false;
  }

  j.transports.resize(2);
  {
    auto& transport = j.transports[0];
    transport.from = 0;
    transport.to = 1;
    transport.train_nr = 1;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[1];
    transport.from = 1;
    transport.to = 2;
    transport.train_nr = 2;
    transport.type = journey::transport::PublicTransport;
  }
  return j;
}

TEST(reliability_connection_graph_builder, journey_train_id_change) {
  connection_graph cg;
  add_base_journey(cg, create_journey_train_id_change());

  ASSERT_EQ(2, cg.stops_.size());
  {
    auto const& stop = cg.stops_[0];
    ASSERT_EQ(stop.index_, 0);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 0);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ(cg.station_info(0).first, "Station1");
    ASSERT_EQ(cg.station_info(0).second, "1111111");
  }
  {
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(stop.index_, 1);
    ASSERT_EQ(stop.alternative_infos_.size(), 0);
    ASSERT_EQ("Station3", cg.station_info(1).first);
    ASSERT_EQ("3333333", cg.station_info(1).second);
  }

  ASSERT_EQ(1, cg.journeys_.size());
  {
    auto const& j = cg.journeys_[0];
    ASSERT_EQ(3, j.stops.size());
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "3333333");
    ASSERT_EQ(2, j.transports.size());
    ASSERT_EQ(j.transports.front().train_nr, 1);
    ASSERT_EQ(j.transports.back().train_nr, 2);
  }
}

journey create_journey_no_new_transport_at_interchange() {
  journey j;
  j.transfers = 1;

  j.stops.resize(3);
  {
    auto& stop = j.stops[0];
    stop.eva_no = "1111111";
    stop.index = 0;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station1";
    stop.arrival.valid = false;
    stop.arrival.timestamp = 1445262900;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445262900;
  }
  {
    auto& stop = j.stops[1];
    stop.eva_no = "2222222";
    stop.index = 1;
    stop.interchange = true;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station2";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263200;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445263500;
  }
  {
    auto& stop = j.stops[2];
    stop.eva_no = "3333333";
    stop.index = 2;
    stop.interchange = false;
    stop.lat = 0.0;
    stop.lng = 0.0;
    stop.name = "Station3";
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445263800;
    stop.departure.valid = false;
  }

  j.transports.resize(1);
  {
    auto& transport = j.transports[0];
    transport.from = 0;
    transport.to = 2;
    transport.train_nr = 1;
    transport.type = journey::transport::PublicTransport;
  }
  return j;
}

TEST(reliability_connection_graph_builder,
     journey_no_new_transport_at_interchange) {
  connection_graph cg;
  add_base_journey(cg, create_journey_no_new_transport_at_interchange());

  ASSERT_EQ(3, cg.stops_.size());
  {
    auto const& stop = cg.stops_[0];
    ASSERT_EQ(stop.index_, 0);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 0);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 2);
    ASSERT_EQ(cg.station_info(0).first, "Station1");
    ASSERT_EQ(cg.station_info(0).second, "1111111");
  }
  {
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(stop.index_, 1);
    ASSERT_EQ(stop.alternative_infos_.size(), 0);
    ASSERT_EQ("Station3", cg.station_info(1).first);
    ASSERT_EQ("3333333", cg.station_info(1).second);
  }
  {
    auto const& stop = cg.stops_[2];
    ASSERT_EQ(stop.index_, 2);
    ASSERT_EQ(stop.alternative_infos_.size(), 1);
    ASSERT_EQ(stop.alternative_infos_[0].journey_index_, 1);
    ASSERT_EQ(stop.alternative_infos_[0].next_stop_index_, 1);
    ASSERT_EQ("Station2", cg.station_info(2).first);
    ASSERT_EQ("2222222", cg.station_info(2).second);
  }

  ASSERT_EQ(2, cg.journeys_.size());
  {
    auto const& j = cg.journeys_[0];
    ASSERT_EQ(2, j.stops.size());
    ASSERT_EQ(j.stops.front().eva_no, "1111111");
    ASSERT_EQ(j.stops.back().eva_no, "2222222");
    ASSERT_EQ(1, j.transports.size());
    ASSERT_EQ(j.transports.front().train_nr, 1);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(2, j.stops.size());
    ASSERT_EQ(j.stops.front().eva_no, "2222222");
    ASSERT_EQ(j.stops.back().eva_no, "3333333");
    ASSERT_EQ(1, j.transports.size());
    ASSERT_EQ(j.transports.front().train_nr, 1);
  }
}

journey create_journey_early_walk() {
  journey j;

  j.stops.resize(6);
  { /* tail of first walk */
    auto& stop = j.stops[0];
    stop.arrival.valid = false;
    stop.departure.valid = true;
    stop.departure.timestamp = 1445212800; /* 00:00 */
    stop.departure.schedule_timestamp = 1445212800; /* 00:00 */
  }
  { /* head of first walk and tail of first train */
    auto& stop = j.stops[1];
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445213400; /* 00:10 */
    stop.arrival.schedule_timestamp = 1445213400; /* 00:10 */
    stop.departure.valid = true;
    stop.departure.timestamp = 1445214000; /* 00:20 */
  }
  { /* head of first train and tail of second walk */
    auto& stop = j.stops[2];
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445214600; /* 00:30 */
    stop.departure.valid = true;
    stop.departure.timestamp = 1445214600; /* 00:30 */
  }
  { /* head of second walk and tail of second train*/
    auto& stop = j.stops[3];
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445215200; /* 00:40 */
    stop.departure.valid = true;
    stop.departure.timestamp = 1445215800; /* 00:50 */
  }
  { /* head of second train and tail of third walk*/
    auto& stop = j.stops[4];
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445216400; /* 01:00 */
    stop.departure.valid = true;
    stop.departure.timestamp = 1445216400; /* 01:00 */
  }
  { /* head of third walk */
    auto& stop = j.stops[5];
    stop.arrival.valid = true;
    stop.arrival.timestamp = 1445217000; /* 01:10 */
    stop.departure.valid = false;
  }

  j.transports.resize(5);
  {
    auto& transport = j.transports[0];
    transport.from = 0;
    transport.to = 1;
    transport.type = journey::transport::Walk;
  }
  {
    auto& transport = j.transports[1];
    transport.from = 1;
    transport.to = 2;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[2];
    transport.from = 2;
    transport.to = 3;
    transport.type = journey::transport::Walk;
  }
  {
    auto& transport = j.transports[3];
    transport.from = 3;
    transport.to = 4;
    transport.type = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports[4];
    transport.from = 4;
    transport.to = 5;
    transport.type = journey::transport::Walk;
  }

  return j;
}

TEST(reliability_connection_graph_builder, move_early_walk) {
  journey const j = detail::move_early_walk(create_journey_early_walk());

  ASSERT_EQ(6, j.stops.size());
  ASSERT_EQ(1445213400 /* 00:10 */, j.stops[0].departure.timestamp);
  ASSERT_EQ(1445213400 /* 00:10 */, j.stops[0].departure.schedule_timestamp);
  ASSERT_EQ(1445214000 /* 00:20 */, j.stops[1].arrival.timestamp);
  ASSERT_EQ(1445214000 /* 00:20 */, j.stops[1].arrival.schedule_timestamp);
  ASSERT_EQ(1445214000 /* 00:20 */, j.stops[1].departure.timestamp);
  ASSERT_EQ(1445214600 /* 00:30 */, j.stops[2].arrival.timestamp);
  ASSERT_EQ(1445215200 /* 00:40 */, j.stops[2].departure.timestamp);
  ASSERT_EQ(1445215800 /* 00:50 */, j.stops[3].arrival.timestamp);
  ASSERT_EQ(1445215800 /* 00:50 */, j.stops[3].departure.timestamp);
  ASSERT_EQ(1445216400 /* 01:00 */, j.stops[4].arrival.timestamp);
  ASSERT_EQ(1445216400 /* 01:00 */, j.stops[4].departure.timestamp);
  ASSERT_EQ(1445217000 /* 01:10 */, j.stops[5].arrival.timestamp);

  ASSERT_EQ(5, j.transports.size());
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
