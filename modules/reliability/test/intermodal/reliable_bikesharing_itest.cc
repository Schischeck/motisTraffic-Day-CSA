#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/journey/message_to_journeys.h"

#include "motis/protocol/BikesharingRequest_generated.h"

#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/intermodal/reliable_bikesharing.h"

#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/schedules/schedule_bikesharing.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {

class reliability_bikesharing : public test_motis_setup {
public:
  reliability_bikesharing()
      : test_motis_setup("modules/bikesharing/test_resources/schedule",
                         "20150112", false, true,
                         "modules/reliability/resources/nextbike") {}
};

class reliability_bikesharing_routing : public test_motis_setup {
public:
  reliability_bikesharing_routing()
      : test_motis_setup(schedule_bikesharing::PATH, schedule_bikesharing::DATE,
                         false, true,
                         "modules/reliability/resources/nextbike_routing") {}
};

TEST_F(reliability_bikesharing, retrieve_bikesharing_infos) {
  auto aggregator = std::make_shared<average_aggregator>(4);
  auto res_dep = call(to_bikesharing_request(true, 49.8776114, 8.6571044,
                                             1454602500, /* Thu, 04 Feb 2016
                                                            16:15:00 GMT */
                                             1454606100, /* Thu, 04 Feb 2016
                                                            17:15:00 GMT */
                                             aggregator->get_aggregator()));
  auto res_arr = call(to_bikesharing_request(false, 50.1273104, 8.6669383,
                                             1454602500, /* Thu, 04 Feb 2016
                                                            16:15:00 GMT */
                                             1454606100, /* Thu, 04 Feb 2016
                                                            17:15:00 GMT */
                                             aggregator->get_aggregator()));

  using ::motis::bikesharing::BikesharingResponse;
  auto dep_infos = detail::to_bikesharing_infos(
      *motis_content(BikesharingResponse, res_dep)->edges(), *aggregator);
  auto arr_infos = detail::to_bikesharing_infos(
      *motis_content(BikesharingResponse, res_arr)->edges(), *aggregator);

  auto sort = [](std::vector<bikesharing_info>& infos) {
    std::sort(infos.begin(), infos.end(), [](bikesharing_info const& a,
                                             bikesharing_info const& b) {
      return a.station_eva_ < b.station_eva_ ||
             (a.station_eva_ == b.station_eva_ && a.duration_ < b.duration_);
    });
  };
  sort(dep_infos);
  sort(arr_infos);

  ASSERT_EQ(4, dep_infos.size());
  {
    auto const& info = dep_infos[0];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(34, info.duration_);
    ASSERT_EQ("Darmstadt Algo", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF East", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = dep_infos[1];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(36, info.duration_);
    ASSERT_EQ("Darmstadt Mensa", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF East", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = dep_infos[2];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(38, info.duration_);
    ASSERT_EQ("Darmstadt Algo", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF West", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = dep_infos[3];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(40, info.duration_);
    ASSERT_EQ("Darmstadt Mensa", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF West", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }

  ASSERT_EQ(4, arr_infos.size());
  {
    auto const& info = arr_infos[0];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(44, info.duration_);
    ASSERT_EQ("FFM HBF North", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 1", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = arr_infos[1];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(45, info.duration_);
    ASSERT_EQ("FFM HBF North", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 2", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = arr_infos[2];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(49, info.duration_);
    ASSERT_EQ("FFM HBF South", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 1", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = arr_infos[3];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(50, info.duration_);
    ASSERT_EQ("FFM HBF South", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 2", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
}

void test_journey1(journey const& j) {
  ASSERT_EQ(4, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ("START", s.eva_no_);
    ASSERT_EQ(1421339100 /* Thu, 15 Jan 2015 16:25:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[1];
    ASSERT_EQ(schedule_bikesharing::DARMSTADT.eva_, s.eva_no_);
    ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ(schedule_bikesharing::FRANKFURT.eva_, s.eva_no_);
    ASSERT_EQ(1421342100 /* Thu, 15 Jan 2015 17:15:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421342400 /* Thu, 15 Jan 2015 17:20:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[3];
    ASSERT_EQ("END", s.eva_no_);
    ASSERT_EQ(1421345520 /* Thu, 15 Jan 2015 18:12:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(3, j.transports_.size());
  ASSERT_TRUE(j.transports_[0].is_walk_);
  ASSERT_FALSE(j.transports_[1].is_walk_);
  ASSERT_TRUE(j.transports_[2].is_walk_);
  ASSERT_EQ(35, j.transports_[0].duration_);
  ASSERT_EQ(15, j.transports_[1].duration_);
  ASSERT_EQ(52, j.transports_[2].duration_);

  ASSERT_EQ(intermodal::BIKESHARING, j.transports_[0].slot_);
  ASSERT_EQ(0, j.transports_[0].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[0].mumo_type_);
  ASSERT_EQ(intermodal::BIKESHARING, j.transports_[2].slot_);
  ASSERT_EQ(0, j.transports_[2].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[2].mumo_type_);
}

void test_journey2(journey const& j) {
  ASSERT_EQ(4, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ("START", s.eva_no_);
    ASSERT_EQ(1421337600 /* Thu, 15 Jan 2015 16:00:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[1];
    ASSERT_EQ(schedule_bikesharing::DARMSTADT.eva_, s.eva_no_);
    ASSERT_EQ(1421339700 /* Thu, 15 Jan 2015 16:35:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421339700 /* Thu, 15 Jan 2015 16:35:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ(schedule_bikesharing::FRANKFURT.eva_, s.eva_no_);
    ASSERT_EQ(1421340000 /* Thu, 15 Jan 2015 16:40:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421340300 /* Thu, 15 Jan 2015 16:45:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[3];
    ASSERT_EQ("END", s.eva_no_);
    ASSERT_EQ(1421344320 /* Thu, 15 Jan 2015 17:52:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(3, j.transports_.size());
  ASSERT_TRUE(j.transports_[0].is_walk_);
  ASSERT_FALSE(j.transports_[1].is_walk_);
  ASSERT_TRUE(j.transports_[2].is_walk_);

  ASSERT_EQ(35, j.transports_[0].duration_);
  ASSERT_EQ(5, j.transports_[1].duration_);
  // ASSERT_EQ(52, j.transports_[2].duration_);

  ASSERT_EQ(intermodal::BIKESHARING, j.transports_[0].slot_);
  ASSERT_EQ(0, j.transports_[0].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[0].mumo_type_);
  ASSERT_EQ(intermodal::BIKESHARING, j.transports_[2].slot_);
  ASSERT_EQ(0, j.transports_[2].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[2].mumo_type_);
}

/* Test a query interval larger than the availability interval */
TEST_F(reliability_bikesharing_routing, large_interval) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421336700, /* 15 Jan 2015 15:45:00 GMT */
                             1421342100 /* 15 Jan 2015 17:15:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true);
  auto res = call(req_msg);
  auto journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, res)->response());

  std::sort(journeys.begin(), journeys.end(),
            [](journey const& a, journey const& b) {
              return a.stops_.front().departure_.schedule_timestamp_ <
                     b.stops_.front().departure_.schedule_timestamp_;
            });

  ASSERT_EQ(2, journeys.size());
  test_journey2(journeys[0]);
  test_journey1(journeys[1]);
}

TEST_F(reliability_bikesharing_routing, rating_request_small_query_interval) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421339100, /* 15 Jan 2015 16:25:00 GMT */
                             1421339100 /* 15 Jan 2015 16:25:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true);
  auto const journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());
  ASSERT_EQ(1, journeys.size());
  test_journey1(journeys[0]);
}

TEST_F(reliability_bikesharing_routing,
       waiting_at_destination_for_bikesharing) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421337600, /* 15 Jan 2015 16:00:00 GMT */
                             1421337660 /* 15 Jan 2015 16:01:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true);
  auto const journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());
  ASSERT_EQ(1, journeys.size());
  test_journey2(journeys[0]);
}

TEST_F(reliability_bikesharing_routing, pretrip_station_to_coordinates) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure Darmstadt Hbf
  // arrival close to campus ffm
  auto req_msg = b.add_pretrip_start(schedule_bikesharing::DARMSTADT.name_,
                                     schedule_bikesharing::DARMSTADT.eva_,
                                     1421339700, /* 15 Jan 2015 16:35:00 GMT */
                                     1421341260 /* 15 Jan 2015 17:01:00 GMT */)
                     .add_intermodal_destination(50.1273104, 8.6669383)
                     .build_rating_request(true);
  auto journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());

  std::sort(journeys.begin(), journeys.end(),
            [](journey const& a, journey const& b) {
              return a.stops_.front().departure_.schedule_timestamp_ <
                     b.stops_.front().departure_.schedule_timestamp_;
            });

  ASSERT_EQ(2, journeys.size());

  {
    auto const& j = journeys[0];

    ASSERT_EQ(3, j.stops_.size());
    {
      auto const& s = j.stops_[0];
      ASSERT_EQ(schedule_bikesharing::DARMSTADT.eva_, s.eva_no_);
      ASSERT_EQ(1421339700 /* Thu, 15 Jan 2015 16:35:00 GMT */,
                s.departure_.schedule_timestamp_);
    }
    {
      auto const& s = j.stops_[1];
      ASSERT_EQ(schedule_bikesharing::FRANKFURT.eva_, s.eva_no_);
      ASSERT_EQ(1421340000 /* Thu, 15 Jan 2015 16:40:00 GMT */,
                s.arrival_.schedule_timestamp_);
      ASSERT_EQ(1421340300 /* Thu, 15 Jan 2015 16:45:00 GMT */,
                s.departure_.schedule_timestamp_);
    }
    {
      auto const& s = j.stops_[2];
      ASSERT_EQ("END", s.eva_no_);
      ASSERT_EQ(1421344320 /* Thu, 15 Jan 2015 17:52:00 GMT */,
                s.arrival_.schedule_timestamp_);
    }

    ASSERT_EQ(2, j.transports_.size());
    ASSERT_FALSE(j.transports_[0].is_walk_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(5, j.transports_[0].duration_);

    ASSERT_EQ(intermodal::BIKESHARING, j.transports_[1].slot_);
    ASSERT_EQ(0, j.transports_[1].mumo_price_);
    ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
              j.transports_[1].mumo_type_);
  }

  {
    auto const& j = journeys[1];

    ASSERT_EQ(3, j.stops_.size());
    {
      auto const& s = j.stops_[0];
      ASSERT_EQ(schedule_bikesharing::DARMSTADT.eva_, s.eva_no_);
      ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
                s.departure_.schedule_timestamp_);
    }
    {
      auto const& s = j.stops_[1];
      ASSERT_EQ(schedule_bikesharing::FRANKFURT.eva_, s.eva_no_);
      ASSERT_EQ(1421342100 /* Thu, 15 Jan 2015 17:15:00 GMT */,
                s.arrival_.schedule_timestamp_);
      ASSERT_EQ(1421342400 /* Thu, 15 Jan 2015 17:20:00 GMT */,
                s.departure_.schedule_timestamp_);
    }
    {
      auto const& s = j.stops_[2];
      ASSERT_EQ("END", s.eva_no_);
      ASSERT_EQ(1421345520 /* Thu, 15 Jan 2015 18:12:00 GMT */,
                s.arrival_.schedule_timestamp_);
    }

    ASSERT_EQ(2, j.transports_.size());
    ASSERT_FALSE(j.transports_[0].is_walk_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(15, j.transports_[0].duration_);
    ASSERT_EQ(52, j.transports_[1].duration_);

    ASSERT_EQ(intermodal::BIKESHARING, j.transports_[1].slot_);
    ASSERT_EQ(0, j.transports_[1].mumo_price_);
    ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
              j.transports_[1].mumo_type_);
  }
}

TEST_F(reliability_bikesharing_routing, ontrip_station_to_coordinates) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure Darmstadt Hbf
  // arrival close to campus ffm
  auto req_msg =
      b.add_ontrip_station_start(schedule_bikesharing::DARMSTADT.name_,
                                 schedule_bikesharing::DARMSTADT.eva_,
                                 1421339700 /* 15 Jan 2015 16:35:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true);
  auto journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());
  ASSERT_EQ(1, journeys.size());

  auto const& j = journeys[0];

  ASSERT_EQ(3, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ(schedule_bikesharing::DARMSTADT.eva_, s.eva_no_);
    ASSERT_EQ(1421339700 /* Thu, 15 Jan 2015 16:35:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[1];
    ASSERT_EQ(schedule_bikesharing::FRANKFURT.eva_, s.eva_no_);
    ASSERT_EQ(1421340000 /* Thu, 15 Jan 2015 16:40:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421340300 /* Thu, 15 Jan 2015 16:45:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ("END", s.eva_no_);
    ASSERT_EQ(1421344320 /* Thu, 15 Jan 2015 17:52:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(2, j.transports_.size());
  ASSERT_FALSE(j.transports_[0].is_walk_);
  ASSERT_TRUE(j.transports_[1].is_walk_);
  ASSERT_EQ(5, j.transports_[0].duration_);

  ASSERT_EQ(intermodal::BIKESHARING, j.transports_[1].slot_);
  ASSERT_EQ(0, j.transports_[1].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[1].mumo_type_);
}

TEST_F(reliability_bikesharing_routing, coordinates_to_station) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421339100, /* 15 Jan 2015 16:25:00 GMT */
                             1421339100 /* 15 Jan 2015 16:25:00 GMT */)
          .add_destination(schedule_bikesharing::FRANKFURT.name_,
                           schedule_bikesharing::FRANKFURT.eva_)
          .build_rating_request(true);
  auto journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());
  ASSERT_EQ(1, journeys.size());

  auto const& j = journeys.front();

  ASSERT_EQ(3, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ("START", s.eva_no_);
    ASSERT_EQ(1421339100 /* Thu, 15 Jan 2015 16:25:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[1];
    ASSERT_EQ(schedule_bikesharing::DARMSTADT.eva_, s.eva_no_);
    ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ(schedule_bikesharing::FRANKFURT.eva_, s.eva_no_);
    ASSERT_EQ(1421342100 /* Thu, 15 Jan 2015 17:15:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(2, j.transports_.size());
  ASSERT_TRUE(j.transports_[0].is_walk_);
  ASSERT_FALSE(j.transports_[1].is_walk_);
  ASSERT_EQ(35, j.transports_[0].duration_);
  ASSERT_EQ(15, j.transports_[1].duration_);

  ASSERT_EQ(intermodal::BIKESHARING, j.transports_[0].slot_);
  ASSERT_EQ(0, j.transports_[0].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[0].mumo_type_);
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
