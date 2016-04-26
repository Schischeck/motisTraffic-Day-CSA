#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/journey/message_to_journeys.h"

#include "motis/protocol/BikesharingRequest_generated.h"

#include "motis/reliability/intermodal/reliable_bikesharing.h"

#include "motis/reliability/tools/flatbuffers/request_builder.h"

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
      : test_motis_setup("modules/reliability/resources/schedule_bikesharing",
                         "20150115", false, true,
                         "modules/reliability/resources/nextbike_routing") {}
};

TEST_F(reliability_bikesharing, test_request) {
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg = to_bikesharing_request(
      49.8776114, 8.6571044, 50.1273104, 8.6669383,
      1454602500, /* Thu, 04 Feb 2016 16:15:00 GMT */
      1454606100, /* Thu, 04 Feb 2016 17:15:00 GMT */
      motis::bikesharing::AvailabilityAggregator_Average);
  auto msg = test::call(motis_instance_, req_msg);
  using ::motis::bikesharing::BikesharingResponse;
  auto const response = motis_content(BikesharingResponse, msg);

  ASSERT_EQ(4, response->departure_edges()->size());
  ASSERT_EQ(4, response->arrival_edges()->size());
}

TEST_F(reliability_bikesharing, retrieve_bikesharing_infos) {
  auto aggregator = std::make_shared<average_aggregator>(4);
  auto infos = retrieve_bikesharing_infos(
      to_bikesharing_request(49.8776114, 8.6571044, 50.1273104, 8.6669383,
                             1454602500, /* Thu, 04 Feb 2016 16:15:00 GMT */
                             1454606100, /* Thu, 04 Feb 2016 17:15:00 GMT */
                             aggregator->get_aggregator()),
      aggregator);

  auto sort = [](std::vector<bikesharing_info>& infos) {
    std::sort(infos.begin(), infos.end(), [](bikesharing_info const& a,
                                             bikesharing_info const& b) {
      return a.station_eva_ < b.station_eva_ ||
             (a.station_eva_ == b.station_eva_ && a.duration_ < b.duration_);
    });
  };
  sort(infos.at_start_);
  sort(infos.at_destination_);

  ASSERT_EQ(4, infos.at_start_.size());
  {
    auto const& info = infos.at_start_[0];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(34, info.duration_);
    ASSERT_EQ("Darmstadt Algo", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF East", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.at_start_[1];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(36, info.duration_);
    ASSERT_EQ("Darmstadt Mensa", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF East", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.at_start_[2];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(38, info.duration_);
    ASSERT_EQ("Darmstadt Algo", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF West", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.at_start_[3];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(40, info.duration_);
    ASSERT_EQ("Darmstadt Mensa", info.from_bike_station_);
    ASSERT_EQ("Darmstadt HBF West", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }

  ASSERT_EQ(4, infos.at_destination_.size());
  {
    auto const& info = infos.at_destination_[0];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(44, info.duration_);
    ASSERT_EQ("FFM HBF North", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 1", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.at_destination_[1];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(45, info.duration_);
    ASSERT_EQ("FFM HBF North", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 2", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.at_destination_[2];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(49, info.duration_);
    ASSERT_EQ("FFM HBF South", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 1", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.at_destination_[3];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(50, info.duration_);
    ASSERT_EQ("FFM HBF South", info.from_bike_station_);
    ASSERT_EQ("FFM Westend 2", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
}

TEST_F(reliability_bikesharing_routing, rating_request) {
  ::motis::reliability::flatbuffers::request_builder::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg = b.add_dep_coordinates(49.8776114, 8.6571044)
                     .add_arr_coordinates(50.1273104, 8.6669383)
                     .set_interval(1421337600, /* 15 Jan 2015 16:00:00 GMT */
                                   1421348400 /* 15 Jan 2015 18:00:00 GMT */)
                     .build_rating_request(true);

  auto const journeys =
      message_to_journeys(motis_content(ReliabilityRatingResponse,
                                        test::call(motis_instance_, req_msg))
                              ->response());
  ASSERT_EQ(1, journeys.size());
  auto const& j = journeys[0];
  ASSERT_EQ(4, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ("-1", s.eva_no_);
    ASSERT_EQ(1421339100, s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[1];
    ASSERT_EQ("Darmstadt Hbf", s.name_);
    ASSERT_EQ(1421341200, s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421341200, s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ("Frankfurt(Main)Hbf", s.name_);
    ASSERT_EQ(1421342100, s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421342400, s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[3];
    ASSERT_EQ("-2", s.eva_no_);
    ASSERT_EQ(1421345520, s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(3, j.transports_.size());
  ASSERT_EQ(journey::transport::Walk, j.transports_[0].type_);
  ASSERT_EQ(journey::transport::PublicTransport, j.transports_[1].type_);
  ASSERT_EQ(journey::transport::Walk, j.transports_[2].type_);
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
