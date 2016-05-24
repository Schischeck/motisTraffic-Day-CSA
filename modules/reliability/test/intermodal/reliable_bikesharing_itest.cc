#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/journey/journey_util.h"
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
  auto msg = call(req_msg);
  using ::motis::bikesharing::BikesharingResponse;
  auto const response = motis_content(BikesharingResponse, msg);

  ASSERT_EQ(4, response->departure_edges()->size());
  ASSERT_EQ(4, response->arrival_edges()->size());
}

TEST_F(reliability_bikesharing, retrieve_bikesharing_infos) {
  auto aggregator = std::make_shared<average_aggregator>(4);
  auto infos = run([&]() {
    return retrieve_bikesharing_infos(
        to_bikesharing_request(49.8776114, 8.6571044, 50.1273104, 8.6669383,
                               1454602500, /* Thu, 04 Feb 2016 16:15:00 GMT */
                               1454606100, /* Thu, 04 Feb 2016 17:15:00 GMT */
                               aggregator->get_aggregator()),
        aggregator);
  });

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
    ASSERT_EQ("Darmstadt Hbf", s.name_);
    ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421341200 /* Thu, 15 Jan 2015 17:00:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ("Frankfurt(Main)Hbf", s.name_);
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
  ASSERT_EQ(journey::transport::Walk, j.transports_[0].type_);
  ASSERT_EQ(journey::transport::PublicTransport, j.transports_[1].type_);
  ASSERT_EQ(journey::transport::Walk, j.transports_[2].type_);
  ASSERT_EQ(35, j.transports_[0].duration_);
  ASSERT_EQ(15, j.transports_[1].duration_);
  ASSERT_EQ(52, j.transports_[2].duration_);
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
    ASSERT_EQ("Darmstadt Hbf", s.name_);
    ASSERT_EQ(1421339700 /* Thu, 15 Jan 2015 16:35:00 GMT */,
              s.arrival_.schedule_timestamp_);
    ASSERT_EQ(1421339700 /* Thu, 15 Jan 2015 16:35:00 GMT */,
              s.departure_.schedule_timestamp_);
  }
  {
    auto const& s = j.stops_[2];
    ASSERT_EQ("Frankfurt(Main)Hbf", s.name_);
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
  ASSERT_EQ(journey::transport::Walk, j.transports_[0].type_);
  ASSERT_EQ(journey::transport::PublicTransport, j.transports_[1].type_);
  ASSERT_EQ(journey::transport::Walk, j.transports_[2].type_);
  ASSERT_EQ(35, j.transports_[0].duration_);
  ASSERT_EQ(5, j.transports_[1].duration_);
  // ASSERT_EQ(52, j.transports_[2].duration_);
}

TEST_F(reliability_bikesharing_routing, rating_request) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421336700, /* 15 Jan 2015 15:45:00 GMT */
                             1421342100 /* 15 Jan 2015 17:15:00 GMT */)
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

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
