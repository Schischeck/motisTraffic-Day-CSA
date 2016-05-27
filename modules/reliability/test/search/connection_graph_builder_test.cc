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
  j.duration_ = 21;
  j.price_ = 10;
  j.transfers_ = 1;

  j.stops_.resize(5);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "0000000";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station0";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445261400;
    stop.departure_.platform_ = "1";
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "1111111";
    stop.interchange_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station1";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262000;
    stop.arrival_.platform_ = "2";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262240;
    stop.departure_.platform_ = "3";
  }
  {
    auto& stop = j.stops_[2];
    stop.eva_no_ = "2222222";
    stop.interchange_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station2";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262900;
    stop.arrival_.platform_ = "4";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262900;
    stop.departure_.platform_ = "";
  }
  {
    auto& stop = j.stops_[3];
    stop.eva_no_ = "3333333";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station3";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263200;
    stop.arrival_.platform_ = "";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445263320;
    stop.departure_.platform_ = "5";
  }
  {
    auto& stop = j.stops_[4];
    stop.eva_no_ = "4444444";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station4";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263920;
    stop.arrival_.platform_ = "6";
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(4);
  {
    auto& transport = j.transports_[0];
    transport.category_id_ = 0;
    transport.category_name_ = "ICE";
    transport.direction_ = "X";
    transport.duration_ = 10;
    transport.from_ = 0;
    transport.line_identifier_ = "l1";
    transport.name_ = "ICE 111";
    transport.provider_ = "DB1";
    transport.slot_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 111;
    transport.type_ = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports_[1];
    transport.category_id_ = 1;
    transport.category_name_ = "IC";
    transport.direction_ = "Y";
    transport.duration_ = 11;
    transport.from_ = 1;
    transport.line_identifier_ = "l2";
    transport.name_ = "IC 222";
    transport.provider_ = "DB2";
    transport.slot_ = 0;
    transport.to_ = 2;
    transport.train_nr_ = 222;
    transport.type_ = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports_[2];
    transport.type_ = journey::transport::Walk;
    transport.duration_ = 5;
    transport.from_ = 2;
    transport.to_ = 3;
    transport.category_id_ = 0;
    transport.category_name_ = "";
    transport.direction_ = "";
    transport.line_identifier_ = "";
    transport.name_ = "";
    transport.provider_ = "";
    transport.slot_ = 0;
    transport.train_nr_ = 0;
  }
  {
    auto& transport = j.transports_[3];
    transport.category_id_ = 3;
    transport.category_name_ = "RB";
    transport.direction_ = "Z";
    transport.duration_ = 10;
    transport.from_ = 3;
    transport.line_identifier_ = "l3";
    transport.name_ = "RB 333";
    transport.provider_ = "DB3";
    transport.slot_ = 0;
    transport.to_ = 4;
    transport.train_nr_ = 333;
    transport.type_ = journey::transport::PublicTransport;
  }

  j.attributes_.resize(4);
  {
    auto& attribute = j.attributes_[0];
    attribute.code_ = "A";
    attribute.from_ = 0;
    attribute.to_ = 1;
    attribute.text_ = "AAA";
  }
  {
    auto& attribute = j.attributes_[1];
    attribute.code_ = "B";
    attribute.from_ = 1;
    attribute.to_ = 2;
    attribute.text_ = "BBB";
  }
  {
    auto& attribute = j.attributes_[2];
    attribute.code_ = "C";
    attribute.from_ = 0;
    attribute.to_ = 2;
    attribute.text_ = "CCC";
  }
  {
    auto& attribute = j.attributes_[3];
    attribute.code_ = "D";
    attribute.from_ = 3;
    attribute.to_ = 4;
    attribute.text_ = "DDD";
  }
  return j;
}

TEST(reliability_connection_graph_builder, split_journey) {
  auto const journeys = detail::split_journey(create_journey1());
  ASSERT_EQ(3, journeys.size());
  {
    auto const& journey = journeys[0];
    ASSERT_EQ(journey.duration_, 10);
    ASSERT_EQ(journey.price_, 0);
    ASSERT_EQ(journey.transfers_, 0);
    ASSERT_EQ(journey.stops_.size(), 2);
    {
      auto& stop = journey.stops_[0];
      ASSERT_EQ(stop.eva_no_, "0000000");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station0");
      ASSERT_FALSE(stop.arrival_.valid_);
      ASSERT_TRUE(stop.departure_.valid_);
      ASSERT_EQ(stop.departure_.timestamp_, 1445261400);
      ASSERT_EQ(stop.departure_.platform_, "1");
    }
    {
      auto& stop = journey.stops_[1];
      ASSERT_EQ(stop.eva_no_, "1111111");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station1");
      ASSERT_TRUE(stop.arrival_.valid_);
      ASSERT_EQ(stop.arrival_.timestamp_, 1445262000);
      ASSERT_EQ(stop.arrival_.platform_, "2");
      ASSERT_FALSE(stop.departure_.valid_);
    }
    ASSERT_EQ(journey.transports_.size(), 1);
    {
      auto& transport = journey.transports_[0];
      ASSERT_EQ(transport.category_id_, 0);
      ASSERT_EQ(transport.category_name_, "ICE");
      ASSERT_EQ(transport.direction_, "X");
      ASSERT_EQ(transport.duration_, 10);
      ASSERT_EQ(transport.from_, 0);
      ASSERT_EQ(transport.line_identifier_, "l1");
      ASSERT_EQ(transport.name_, "ICE 111");
      ASSERT_EQ(transport.provider_, "DB1");
      ASSERT_EQ(transport.slot_, 0);
      ASSERT_EQ(transport.to_, 1);
      ASSERT_EQ(transport.train_nr_, 111);
      ASSERT_EQ(journey::transport::PublicTransport, transport.type_);
    }
    ASSERT_EQ(journey.attributes_.size(), 2);
    {
      auto& attribute = journey.attributes_[0];
      ASSERT_EQ(attribute.code_, "A");
      ASSERT_EQ(attribute.from_, 0);
      ASSERT_EQ(attribute.to_, 1);
      ASSERT_EQ(attribute.text_, "AAA");
    }
    {
      auto& attribute = journey.attributes_[1];
      ASSERT_EQ(attribute.code_, "C");
      ASSERT_EQ(attribute.from_, 0);
      ASSERT_EQ(attribute.to_, 1);
      ASSERT_EQ(attribute.text_, "CCC");
    }
  }
  {
    auto const& journey = journeys[1];
    ASSERT_EQ(journey.duration_, 11);
    ASSERT_EQ(journey.price_, 0);
    ASSERT_EQ(journey.transfers_, 0);
    ASSERT_EQ(journey.stops_.size(), 2);
    {
      auto& stop = journey.stops_[0];
      ASSERT_EQ(stop.eva_no_, "1111111");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station1");
      ASSERT_FALSE(stop.arrival_.valid_);
      ASSERT_TRUE(stop.departure_.valid_);
      ASSERT_EQ(stop.departure_.timestamp_, 1445262240);
      ASSERT_EQ(stop.departure_.platform_, "3");
    }
    {
      auto& stop = journey.stops_[1];
      ASSERT_EQ(stop.eva_no_, "2222222");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station2");
      ASSERT_TRUE(stop.arrival_.valid_);
      ASSERT_EQ(stop.arrival_.timestamp_, 1445262900);
      ASSERT_EQ(stop.arrival_.platform_, "4");
      ASSERT_FALSE(stop.departure_.valid_);
    }
    ASSERT_EQ(journey.transports_.size(), 1);
    {
      auto& transport = journey.transports_[0];
      ASSERT_EQ(transport.category_id_, 1);
      ASSERT_EQ(transport.category_name_, "IC");
      ASSERT_EQ(transport.direction_, "Y");
      ASSERT_EQ(transport.duration_, 11);
      ASSERT_EQ(transport.from_, 0);
      ASSERT_EQ(transport.line_identifier_, "l2");
      ASSERT_EQ(transport.name_, "IC 222");
      ASSERT_EQ(transport.provider_, "DB2");
      ASSERT_EQ(transport.slot_, 0);
      ASSERT_EQ(transport.to_, 1);
      ASSERT_EQ(transport.train_nr_, 222);
      ASSERT_EQ(journey::transport::PublicTransport, transport.type_);
    }
    ASSERT_EQ(journey.attributes_.size(), 2);
    {
      auto& attribute = journey.attributes_[0];
      ASSERT_EQ(attribute.code_, "B");
      ASSERT_EQ(attribute.from_, 0);
      ASSERT_EQ(attribute.to_, 1);
      ASSERT_EQ(attribute.text_, "BBB");
    }
    {
      auto& attribute = journey.attributes_[1];
      ASSERT_EQ(attribute.code_, "C");
      ASSERT_EQ(attribute.from_, 0);
      ASSERT_EQ(attribute.to_, 1);
      ASSERT_EQ(attribute.text_, "CCC");
    }
  }
  {
    auto const& journey = journeys[2];
    ASSERT_EQ(journey.duration_, 17);
    ASSERT_EQ(journey.price_, 0);
    ASSERT_EQ(journey.transfers_, 0);
    ASSERT_EQ(journey.stops_.size(), 3);
    {
      auto& stop = journey.stops_[0];
      ASSERT_EQ(stop.eva_no_, "2222222");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station2");
      ASSERT_FALSE(stop.arrival_.valid_);
      ASSERT_TRUE(stop.departure_.valid_);
      ASSERT_EQ(stop.departure_.timestamp_, 1445262900);
      ASSERT_EQ(stop.departure_.platform_, "");
    }
    {
      auto& stop = journey.stops_[1];
      ASSERT_EQ(stop.eva_no_, "3333333");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station3");
      ASSERT_TRUE(stop.arrival_.valid_);
      ASSERT_EQ(stop.arrival_.timestamp_, 1445263200);
      ASSERT_EQ(stop.arrival_.platform_, "");
      ASSERT_TRUE(stop.departure_.valid_);
      ASSERT_EQ(stop.departure_.timestamp_, 1445263320);
      ASSERT_EQ(stop.departure_.platform_, "5");
    }
    {
      auto& stop = journey.stops_[2];
      ASSERT_EQ(stop.eva_no_, "4444444");
      ASSERT_FALSE(stop.interchange_);
      ASSERT_EQ(stop.lat_, 0.0);
      ASSERT_EQ(stop.lng_, 0.0);
      ASSERT_EQ(stop.name_, "Station4");
      ASSERT_TRUE(stop.arrival_.valid_);
      ASSERT_EQ(stop.arrival_.timestamp_, 1445263920);
      ASSERT_EQ(stop.arrival_.platform_, "6");
      ASSERT_FALSE(stop.departure_.valid_);
    }
    ASSERT_EQ(journey.transports_.size(), 2);
    {
      auto& transport = journey.transports_[0];
      ASSERT_EQ(journey::transport::Walk, transport.type_);
      ASSERT_EQ(transport.duration_, 5);
      ASSERT_EQ(transport.from_, 0);
      ASSERT_EQ(transport.to_, 1);
      ASSERT_EQ(transport.category_id_, 0);
      ASSERT_EQ(transport.category_name_, "");
      ASSERT_EQ(transport.direction_, "");
      ASSERT_EQ(transport.line_identifier_, "");
      ASSERT_EQ(transport.name_, "");
      ASSERT_EQ(transport.provider_, "");
      ASSERT_EQ(transport.slot_, 0);
      ASSERT_EQ(transport.train_nr_, 0);
    }
    {
      auto& transport = journey.transports_[1];
      ASSERT_EQ(transport.category_id_, 3);
      ASSERT_EQ(transport.category_name_, "RB");
      ASSERT_EQ(transport.direction_, "Z");
      ASSERT_EQ(transport.duration_, 10);
      ASSERT_EQ(transport.from_, 1);
      ASSERT_EQ(transport.line_identifier_, "l3");
      ASSERT_EQ(transport.name_, "RB 333");
      ASSERT_EQ(transport.provider_, "DB3");
      ASSERT_EQ(transport.slot_, 0);
      ASSERT_EQ(transport.to_, 2);
      ASSERT_EQ(transport.train_nr_, 333);
      ASSERT_EQ(journey::transport::PublicTransport, transport.type_);
    }
    ASSERT_EQ(journey.attributes_.size(), 1);
    {
      auto& attribute = journey.attributes_[0];
      ASSERT_EQ(attribute.code_, "D");
      ASSERT_EQ(attribute.from_, 1);
      ASSERT_EQ(attribute.to_, 2);
      ASSERT_EQ(attribute.text_, "DDD");
    }
  }
}

journey create_journey2() {
  journey j;
  j.duration_ = 20;
  j.price_ = 0;
  j.transfers_ = 0;

  j.stops_.resize(2);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "0000000";
    stop.name_ = "Station0";
    stop.interchange_ = false;
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445261400;
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "1111111";
    stop.name_ = "Station1";
    stop.interchange_ = false;
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262600;
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(1);
  {
    auto& transport = j.transports_[0];
    transport.duration_ = 10;
    transport.from_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 111;
    transport.type_ = journey::transport::PublicTransport;
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
    ASSERT_EQ(j.stops_.front().eva_no_, "0000000");
    ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
    ASSERT_EQ(j.transports_.front().train_nr_, 111);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "2222222");
    ASSERT_EQ(j.transports_.front().train_nr_, 222);
  }
  {
    auto const& j = cg.journeys_[2];
    ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.back().eva_no_, "4444444");
    ASSERT_EQ(j.transports_.back().train_nr_, 333);
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
    ASSERT_EQ(j.stops_.front().eva_no_, "0000000");
    ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
    ASSERT_EQ(j.transports_.front().train_nr_, 111);
  }
}

journey create_journey3() {
  journey j;
  j.transfers_ = 0;

  j.stops_.resize(2);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station1";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262600;
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "4444444";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station4";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445264400;
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(1);
  {
    auto& transport = j.transports_[0];
    transport.from_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 555;
    transport.type_ = journey::transport::PublicTransport;
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
    ASSERT_EQ(j.stops_.front().eva_no_, "0000000");
    ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
    ASSERT_EQ(j.transports_.front().train_nr_, 111);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "2222222");
    ASSERT_EQ(j.transports_.front().train_nr_, 222);
  }
  {
    auto const& j = cg.journeys_[2];
    ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.back().eva_no_, "4444444");
    ASSERT_EQ(j.transports_.back().train_nr_, 333);
  }
  {
    auto const& j = cg.journeys_[3];
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "4444444");
    ASSERT_EQ(j.transports_.back().train_nr_, 555);
  }
}

journey create_journey4() {
  journey j;
  j.transfers_ = 1;

  j.stops_.resize(3);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station1";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262900;
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "5555555";
    stop.interchange_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station5";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263200;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445263500;
  }
  {
    auto& stop = j.stops_[2];
    stop.eva_no_ = "4444444";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station4";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263800;
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(2);
  {
    auto& transport = j.transports_[0];
    transport.from_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 666;
    transport.type_ = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports_[1];
    transport.from_ = 1;
    transport.to_ = 2;
    transport.train_nr_ = 777;
    transport.type_ = journey::transport::PublicTransport;
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
    ASSERT_EQ(j.stops_.front().eva_no_, "0000000");
    ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
    ASSERT_EQ(j.transports_.front().train_nr_, 111);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "2222222");
    ASSERT_EQ(j.transports_.front().train_nr_, 222);
  }
  {
    auto const& j = cg.journeys_[2];
    ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.back().eva_no_, "4444444");
    ASSERT_EQ(j.transports_.back().train_nr_, 333);
  }
  {
    auto const& j = cg.journeys_[3];
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "4444444");
    ASSERT_EQ(j.transports_.back().train_nr_, 555);
  }
  {
    auto const& j = cg.journeys_[4];
    ASSERT_EQ("1111111", j.stops_.front().eva_no_);
    ASSERT_EQ("5555555", j.stops_.back().eva_no_);
    ASSERT_EQ(666, j.transports_.back().train_nr_);
  }
  {
    auto const& j = cg.journeys_[5];
    ASSERT_EQ(j.stops_.front().eva_no_, "5555555");
    ASSERT_EQ(j.stops_.back().eva_no_, "4444444");
    ASSERT_EQ(j.transports_.back().train_nr_, 777);
  }
}

journey create_journey_train_id_change() {
  journey j;
  j.transfers_ = 1;

  j.stops_.resize(3);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station1";
    stop.arrival_.valid_ = false;
    stop.arrival_.timestamp_ = 1445262900;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262900;
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "2222222";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station2";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263200;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445263500;
  }
  {
    auto& stop = j.stops_[2];
    stop.eva_no_ = "3333333";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station3";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263800;
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(2);
  {
    auto& transport = j.transports_[0];
    transport.from_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 1;
    transport.type_ = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports_[1];
    transport.from_ = 1;
    transport.to_ = 2;
    transport.train_nr_ = 2;
    transport.type_ = journey::transport::PublicTransport;
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
    ASSERT_EQ(3, j.stops_.size());
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "3333333");
    ASSERT_EQ(2, j.transports_.size());
    ASSERT_EQ(j.transports_.front().train_nr_, 1);
    ASSERT_EQ(j.transports_.back().train_nr_, 2);
  }
}

journey create_journey_no_new_transport_at_interchange() {
  journey j;
  j.transfers_ = 1;

  j.stops_.resize(3);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station1";
    stop.arrival_.valid_ = false;
    stop.arrival_.timestamp_ = 1445262900;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262900;
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "2222222";
    stop.interchange_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station2";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263200;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445263500;
  }
  {
    auto& stop = j.stops_[2];
    stop.eva_no_ = "3333333";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Station3";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263800;
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(1);
  {
    auto& transport = j.transports_[0];
    transport.from_ = 0;
    transport.to_ = 2;
    transport.train_nr_ = 1;
    transport.type_ = journey::transport::PublicTransport;
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
    ASSERT_EQ(2, j.stops_.size());
    ASSERT_EQ(j.stops_.front().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.back().eva_no_, "2222222");
    ASSERT_EQ(1, j.transports_.size());
    ASSERT_EQ(j.transports_.front().train_nr_, 1);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(2, j.stops_.size());
    ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.back().eva_no_, "3333333");
    ASSERT_EQ(1, j.transports_.size());
    ASSERT_EQ(j.transports_.front().train_nr_, 1);
  }
}

journey create_journey_early_walk() {
  journey j;

  j.stops_.resize(6);
  { /* tail of first walk */
    auto& stop = j.stops_[0];
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445212800; /* 00:00 */
    stop.departure_.schedule_timestamp_ = 1445212800; /* 00:00 */
  }
  { /* head of first walk and tail of first train */
    auto& stop = j.stops_[1];
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445213400; /* 00:10 */
    stop.arrival_.schedule_timestamp_ = 1445213400; /* 00:10 */
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445214000; /* 00:20 */
  }
  { /* head of first train and tail of second walk */
    auto& stop = j.stops_[2];
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445214600; /* 00:30 */
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445214600; /* 00:30 */
  }
  { /* head of second walk and tail of second train*/
    auto& stop = j.stops_[3];
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445215200; /* 00:40 */
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445215800; /* 00:50 */
  }
  { /* head of second train and tail of third walk*/
    auto& stop = j.stops_[4];
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445216400; /* 01:00 */
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445216400; /* 01:00 */
  }
  { /* head of third walk */
    auto& stop = j.stops_[5];
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445217000; /* 01:10 */
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(5);
  {
    auto& transport = j.transports_[0];
    transport.from_ = 0;
    transport.to_ = 1;
    transport.type_ = journey::transport::Walk;
  }
  {
    auto& transport = j.transports_[1];
    transport.from_ = 1;
    transport.to_ = 2;
    transport.type_ = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports_[2];
    transport.from_ = 2;
    transport.to_ = 3;
    transport.type_ = journey::transport::Walk;
  }
  {
    auto& transport = j.transports_[3];
    transport.from_ = 3;
    transport.to_ = 4;
    transport.type_ = journey::transport::PublicTransport;
  }
  {
    auto& transport = j.transports_[4];
    transport.from_ = 4;
    transport.to_ = 5;
    transport.type_ = journey::transport::Walk;
  }

  return j;
}

TEST(reliability_connection_graph_builder, move_early_walk) {
  journey const j = detail::move_early_walk(create_journey_early_walk());

  ASSERT_EQ(6, j.stops_.size());
  ASSERT_EQ(1445213400 /* 00:10 */, j.stops_[0].departure_.timestamp_);
  ASSERT_EQ(1445213400 /* 00:10 */, j.stops_[0].departure_.schedule_timestamp_);
  ASSERT_EQ(1445214000 /* 00:20 */, j.stops_[1].arrival_.timestamp_);
  ASSERT_EQ(1445214000 /* 00:20 */, j.stops_[1].arrival_.schedule_timestamp_);
  ASSERT_EQ(1445214000 /* 00:20 */, j.stops_[1].departure_.timestamp_);
  ASSERT_EQ(1445214600 /* 00:30 */, j.stops_[2].arrival_.timestamp_);
  ASSERT_EQ(1445215200 /* 00:40 */, j.stops_[2].departure_.timestamp_);
  ASSERT_EQ(1445215800 /* 00:50 */, j.stops_[3].arrival_.timestamp_);
  ASSERT_EQ(1445215800 /* 00:50 */, j.stops_[3].departure_.timestamp_);
  ASSERT_EQ(1445216400 /* 01:00 */, j.stops_[4].arrival_.timestamp_);
  ASSERT_EQ(1445216400 /* 01:00 */, j.stops_[4].departure_.timestamp_);
  ASSERT_EQ(1445217000 /* 01:10 */, j.stops_[5].arrival_.timestamp_);

  ASSERT_EQ(5, j.transports_.size());
}

}  // namespace connection_graph_builder
}  // namespace search
}  // namespace reliability
}  // namespace motis
