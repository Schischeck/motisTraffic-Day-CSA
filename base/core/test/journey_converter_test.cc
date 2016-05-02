#include "gtest/gtest.h"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/core/schedule/category.h"

using namespace motis;
using routing::RoutingResponse;

journey create_journey1() {
  journey j;
  j.duration_ = 30;
  j.price_ = 10;
  j.transfers_ = 1;

  j.stops_.resize(4);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop0";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445261400;
    stop.departure_.platform_ = "1";
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "2222222";
    stop.interchange_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop1";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262000;
    stop.arrival_.platform_ = "2";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262240;
    stop.departure_.platform_ = "3";
  }
  {
    auto& stop = j.stops_[2];
    stop.eva_no_ = "3333333";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop2";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262900;
    stop.arrival_.platform_ = "4";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262900;
    stop.departure_.platform_ = "";
  }
  {
    auto& stop = j.stops_[3];
    stop.eva_no_ = "4444444";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop3";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263200;
    stop.arrival_.platform_ = "";
    stop.departure_.valid_ = false;
  }

  j.transports_.resize(3);
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

  j.attributes_.resize(2);
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
  return j;
}

journey create_journey2() {
  journey j;
  j.duration_ = 15;
  j.price_ = 10;
  j.transfers_ = 0;

  j.stops_.resize(2);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop0";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445328000;
    stop.departure_.platform_ = "1";
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "2222222";
    stop.interchange_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop1";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445328900;
    stop.arrival_.platform_ = "2";
    stop.departure_.valid_ = false;
    stop.departure_.timestamp_ = 0;
    stop.departure_.platform_ = "3";
  }
  j.transports_.resize(1);
  {
    auto& transport = j.transports_[0];
    transport.category_id_ = 0;
    transport.category_name_ = "ICE";
    transport.direction_ = "X";
    transport.duration_ = 15;
    transport.from_ = 0;
    transport.line_identifier_ = "l1";
    transport.name_ = "ICE 111";
    transport.provider_ = "DB1";
    transport.slot_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 111;
    transport.type_ = journey::transport::PublicTransport;
  }
  return j;
}

TEST(core_convert_journey, journey_message_journey) {
  std::vector<journey> original_journeys;
  original_journeys.push_back(create_journey1());
  original_journeys.push_back(create_journey2());

  auto msg = journeys_to_message(original_journeys);
  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_TRUE(journeys.size() == 2);

  for (unsigned int i = 0; i < 2; ++i) {
    auto const& j = journeys[i];
    auto const& o = original_journeys[i];

    ASSERT_TRUE(o.duration_ == j.duration_);
    // ASSERT_TRUE(o.price_ == j.price_); TODO(Mohammad Keyhani)
    ASSERT_TRUE(o.transfers_ == j.transfers_);
    ASSERT_TRUE(o.stops_.size() == j.stops_.size());
    ASSERT_TRUE(o.transports_.size() == j.transports_.size());
    ASSERT_TRUE(o.attributes_.size() == j.attributes_.size());

    for (unsigned int s = 0; s < o.stops_.size(); ++s) {
      auto const& os = o.stops_[s];
      auto const& js = j.stops_[s];
      ASSERT_EQ(os.eva_no_, js.eva_no_);
      ASSERT_EQ(os.interchange_, js.interchange_);
      ASSERT_EQ(os.lat_, js.lat_);
      ASSERT_EQ(os.lng_, js.lng_);
      ASSERT_EQ(os.name_, js.name_);
      ASSERT_EQ(os.arrival_.platform_, js.arrival_.platform_);
      ASSERT_EQ(os.arrival_.timestamp_, js.arrival_.timestamp_);
      ASSERT_EQ(os.arrival_.valid_, js.arrival_.valid_);
      ASSERT_EQ(os.departure_.platform_, js.departure_.platform_);
      ASSERT_EQ(os.departure_.timestamp_, js.departure_.timestamp_);
      ASSERT_EQ(os.departure_.valid_, js.departure_.valid_);
    }

    for (unsigned int t = 0; t < o.transports_.size(); ++t) {
      auto const& ot = o.transports_[t];
      auto const& jt = j.transports_[t];
      ASSERT_EQ(ot.category_id_, jt.category_id_);
      ASSERT_EQ(ot.category_name_, jt.category_name_);
      ASSERT_EQ(ot.direction_, jt.direction_);
      ASSERT_EQ(ot.duration_, jt.duration_);
      ASSERT_EQ(ot.from_, jt.from_);
      ASSERT_EQ(ot.line_identifier_, jt.line_identifier_);
      ASSERT_EQ(ot.name_, jt.name_);
      ASSERT_EQ(ot.provider_, jt.provider_);
      ASSERT_EQ(ot.slot_, jt.slot_);
      ASSERT_EQ(ot.to_, jt.to_);
      ASSERT_EQ(ot.train_nr_, jt.train_nr_);
      ASSERT_EQ(ot.type_, jt.type_);
    }

    for (unsigned int a = 0; a < o.attributes_.size(); ++a) {
      auto const& oa = o.attributes_[a];
      auto const& ja = j.attributes_[a];
      ASSERT_EQ(oa.code_, ja.code_);
      ASSERT_EQ(oa.from_, ja.from_);
      ASSERT_EQ(oa.to_, ja.to_);
      ASSERT_EQ(oa.text_, ja.text_);
    }
  }
}
