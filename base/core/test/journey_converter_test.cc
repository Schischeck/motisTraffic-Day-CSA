#include "gtest/gtest.h"

#include "motis/core/common/transform_to_vec.h"
#include "motis/core/schedule/category.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/journey/message_to_journeys.h"

using namespace motis;
using namespace motis::module;
using routing::RoutingResponse;

msg_ptr journeys_to_message(std::vector<journey> const& journeys) {
  message_creator fbb;
  routing::Statistics stats(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  fbb.create_and_finish(
      MsgContent_RoutingResponse,
      routing::CreateRoutingResponse(
          fbb, &stats, fbb.CreateVector(transform_to_vec(journeys,
                                                         [&](journey const& j) {
                                                           return to_connection(
                                                               fbb, j);
                                                         })))
          .Union());
  return make_msg(fbb);
}

journey create_journey1() {
  journey j;
  j.duration_ = 30;
  j.price_ = 10;
  j.transfers_ = 1;
  j.db_costs_ = 100;
  j.night_penalty_ = 200;

  j.stops_.resize(4);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.enter_ = true;
    stop.leave_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop0";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445261400;
    stop.departure_.track_ = "1";
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "2222222";
    stop.enter_ = true;
    stop.leave_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop1";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262000;
    stop.arrival_.track_ = "2";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262240;
    stop.departure_.track_ = "3";
  }
  {
    auto& stop = j.stops_[2];
    stop.eva_no_ = "3333333";
    stop.enter_ = true;
    stop.leave_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop2";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445262900;
    stop.arrival_.track_ = "4";
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445262900;
    stop.departure_.track_ = "";
  }
  {
    auto& stop = j.stops_[3];
    stop.eva_no_ = "4444444";
    stop.enter_ = false;
    stop.leave_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop3";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445263200;
    stop.arrival_.track_ = "";
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
    transport.mumo_id_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 111;
    transport.is_walk_ = false;
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
    transport.mumo_id_ = 0;
    transport.to_ = 2;
    transport.train_nr_ = 222;
    transport.is_walk_ = false;
  }
  {
    auto& transport = j.transports_[2];
    transport.is_walk_ = true;
    transport.duration_ = 5;
    transport.from_ = 2;
    transport.to_ = 3;
    transport.category_id_ = 0;
    transport.category_name_ = "";
    transport.direction_ = "";
    transport.line_identifier_ = "";
    transport.name_ = "";
    transport.provider_ = "";
    transport.mumo_id_ = 0;
    transport.train_nr_ = 0;
  }

  j.trips_.resize(3);
  {
    auto& trip = j.trips_[0];
    trip.from_ = 0;
    trip.to_ = 2;
    trip.station_id_ = "S";
    trip.train_nr_ = 1;
    trip.time_ = 1445261200;
    trip.target_station_id_ = "T";
    trip.target_time_ = 1445231200;
    trip.line_id_ = "1234";
  }
  {
    auto& trip = j.trips_[1];
    trip.from_ = 0;
    trip.to_ = 2;
    trip.station_id_ = "X";
    trip.train_nr_ = 2;
    trip.time_ = 1445261201;
    trip.target_station_id_ = "Y";
    trip.target_time_ = 1445231202;
    trip.line_id_ = "4321";
  }
  {
    auto& trip = j.trips_[2];
    trip.from_ = 1;
    trip.to_ = 2;
    trip.station_id_ = "A";
    trip.train_nr_ = 3;
    trip.time_ = 1445261203;
    trip.target_station_id_ = "B";
    trip.target_time_ = 1445231204;
    trip.line_id_ = "0";
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
  j.db_costs_ = 100;
  j.night_penalty_ = 200;

  j.stops_.resize(2);
  {
    auto& stop = j.stops_[0];
    stop.eva_no_ = "1111111";
    stop.enter_ = true;
    stop.leave_ = false;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop0";
    stop.arrival_.valid_ = false;
    stop.departure_.valid_ = true;
    stop.departure_.timestamp_ = 1445328000;
    stop.departure_.track_ = "1";
  }
  {
    auto& stop = j.stops_[1];
    stop.eva_no_ = "2222222";
    stop.enter_ = false;
    stop.leave_ = true;
    stop.lat_ = 0.0;
    stop.lng_ = 0.0;
    stop.name_ = "Stop1";
    stop.arrival_.valid_ = true;
    stop.arrival_.timestamp_ = 1445328900;
    stop.arrival_.track_ = "2";
    stop.departure_.valid_ = false;
    stop.departure_.timestamp_ = 0;
    stop.departure_.track_ = "3";
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
    transport.mumo_id_ = 0;
    transport.to_ = 1;
    transport.train_nr_ = 111;
    transport.is_walk_ = false;
  }
  j.trips_.resize(1);
  {
    auto& trip = j.trips_[0];
    trip.from_ = 0;
    trip.to_ = 2;
    trip.station_id_ = "S";
    trip.train_nr_ = 1;
    trip.time_ = 1445261200;
    trip.target_station_id_ = "T";
    trip.target_time_ = 1445231200;
    trip.line_id_ = "1234";
  }
  return j;
}

TEST(core_convert_journey, journey_message_journey) {
  std::vector<journey> original_journeys = {create_journey1(),
                                            create_journey2()};

  auto msg = journeys_to_message(original_journeys);
  auto journeys = message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_TRUE(journeys.size() == 2);

  for (unsigned int i = 0; i < 2; ++i) {
    auto const& j = journeys[i];
    auto const& o = original_journeys[i];

    ASSERT_EQ(o.duration_, j.duration_);
    // ASSERT_TRUE(o.price_ == j.price_); TODO(Mohammad Keyhani)
    EXPECT_EQ(o.transfers_, j.transfers_);
    EXPECT_EQ(o.stops_.size(), j.stops_.size());
    EXPECT_EQ(o.transports_.size(), j.transports_.size());
    EXPECT_EQ(o.attributes_.size(), j.attributes_.size());

    for (unsigned int s = 0; s < o.stops_.size(); ++s) {
      auto const& os = o.stops_[s];
      auto const& js = j.stops_[s];
      ASSERT_EQ(os.eva_no_, js.eva_no_);
      ASSERT_EQ(os.enter_, js.enter_);
      ASSERT_EQ(os.leave_, js.leave_);
      ASSERT_EQ(os.lat_, js.lat_);
      ASSERT_EQ(os.lng_, js.lng_);
      ASSERT_EQ(os.name_, js.name_);
      ASSERT_EQ(os.arrival_.valid_, js.arrival_.valid_);
      ASSERT_EQ(os.departure_.valid_, js.departure_.valid_);
      if (os.arrival_.valid_) {
        ASSERT_EQ(os.arrival_.track_, js.arrival_.track_);
        ASSERT_EQ(os.arrival_.timestamp_, js.arrival_.timestamp_);
        ASSERT_EQ(os.arrival_.valid_, js.arrival_.valid_);
      }
      if (os.departure_.valid_) {
        ASSERT_EQ(os.departure_.track_, js.departure_.track_);
        ASSERT_EQ(os.departure_.timestamp_, js.departure_.timestamp_);
        ASSERT_EQ(os.departure_.valid_, js.departure_.valid_);
      }
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
      ASSERT_EQ(ot.mumo_id_, jt.mumo_id_);
      ASSERT_EQ(ot.to_, jt.to_);
      ASSERT_EQ(ot.train_nr_, jt.train_nr_);
      ASSERT_EQ(ot.is_walk_, jt.is_walk_);
      ASSERT_EQ(ot.mumo_price_, jt.mumo_price_);
      ASSERT_EQ(ot.mumo_type_, jt.mumo_type_);
    }

    for (unsigned int s = 0; s < o.trips_.size(); ++s) {
      auto const& ot = o.trips_[s];
      auto const& jt = j.trips_[s];
      ASSERT_EQ(ot.from_, jt.from_);
      ASSERT_EQ(ot.to_, jt.to_);
      ASSERT_EQ(ot.station_id_, jt.station_id_);
      ASSERT_EQ(ot.train_nr_, jt.train_nr_);
      ASSERT_EQ(ot.time_, jt.time_);
      ASSERT_EQ(ot.target_station_id_, jt.target_station_id_);
      ASSERT_EQ(ot.target_time_, jt.target_time_);
      ASSERT_EQ(ot.line_id_, jt.line_id_);
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
