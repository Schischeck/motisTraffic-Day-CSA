#include "gtest/gtest.h"

#include <cstdlib>
#include <iostream>

#include "motis/core/common/constants.h"
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

TEST_F(reliability_bikesharing, compress_intervals) {
  auto create = [](time_t const& from, time_t const& to) {
    return bikesharing_info::availability{from, to, 0};
  };
  {
    std::vector<bikesharing_info::availability> intervals = {
        create(1465293600, 1465297200),
        create(1465297200, 1465300800),
        create(1465304400, 1465308000),
        create(1465311600, 1465315200),
        create(1465315200, 1465318800),
        bikesharing_info::availability{1465318800, 1465318860, 1}};
    auto const c = detail::compress_intervals(intervals);
    ASSERT_EQ(4, c.size());
    ASSERT_EQ(1465293600, c[0].from_);
    ASSERT_EQ(1465300800, c[0].to_);
    ASSERT_EQ(1465304400, c[1].from_);
    ASSERT_EQ(1465308000, c[1].to_);
    ASSERT_EQ(1465311600, c[2].from_);
    ASSERT_EQ(1465318800, c[2].to_);
    ASSERT_EQ(1465318800, c[3].from_);
    ASSERT_EQ(1465318860, c[3].to_);

    ASSERT_EQ(0, c[0].rating_);
    ASSERT_EQ(1, c[3].rating_);
  }
  {
    std::vector<bikesharing_info::availability> intervals = {
        create(1465293600, 1465297200), create(1465304400, 1465308000),
        create(1465311600, 1465315200), create(1465315200, 1465318800)};
    auto const c = detail::compress_intervals(intervals);
    ASSERT_EQ(3, c.size());
    ASSERT_EQ(1465293600, c[0].from_);
    ASSERT_EQ(1465297200, c[0].to_);
    ASSERT_EQ(1465304400, c[1].from_);
    ASSERT_EQ(1465308000, c[1].to_);
    ASSERT_EQ(1465311600, c[2].from_);
    ASSERT_EQ(1465318800, c[2].to_);
  }
}

TEST_F(reliability_bikesharing, pareto_filter) {
  std::vector<bikesharing_info> all;
  auto b = [&all](unsigned bike, unsigned walk, std::string station) {
    all.push_back({bike,
                   walk,
                   station,
                   bikesharing_info::terminal{0.0, 0.0},
                   bikesharing_info::terminal{0.0, 0.0},
                   {}});
  };
  b(80, 20, "A");
  b(10, 10, "A");
  b(20, 5, "A");
  b(16, 5, "A");
  b(20, 20, "B");

  auto filtered = detail::pareto_filter(all);
  std::sort(filtered.begin(), filtered.end(), [](auto const& a, auto const& b) {
    return std::make_pair(a.bike_duration_, a.walk_duration_) <
           std::make_pair(b.bike_duration_, b.walk_duration_);
  });

  ASSERT_EQ(4, filtered.size());
  ASSERT_EQ(10, filtered[0].bike_duration_);
  ASSERT_EQ(10, filtered[0].walk_duration_);
  ASSERT_EQ(16, filtered[1].bike_duration_);
  ASSERT_EQ(5, filtered[1].walk_duration_);
  ASSERT_EQ(20, filtered[2].bike_duration_);
  ASSERT_EQ(5, filtered[2].walk_duration_);
  ASSERT_EQ(20, filtered[3].bike_duration_);
  ASSERT_EQ(20, filtered[3].walk_duration_);
}

TEST_F(reliability_bikesharing, DISABLED_retrieve_bikesharing_infos) {
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
      *motis_content(BikesharingResponse, res_dep)->edges(), *aggregator, 120);
  auto arr_infos = detail::to_bikesharing_infos(
      *motis_content(BikesharingResponse, res_arr)->edges(), *aggregator, 120);

  auto sort = [](std::vector<bikesharing_info>& infos) {
    std::sort(infos.begin(), infos.end(), [](bikesharing_info const& a,
                                             bikesharing_info const& b) {
      return a.station_eva_ < b.station_eva_ ||
             (a.station_eva_ == b.station_eva_ && a.duration() < b.duration());
    });
  };
  sort(dep_infos);
  sort(arr_infos);

  ASSERT_EQ(4, dep_infos.size());
  {
    auto const& info = dep_infos[0];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(17, info.duration());
    // ASSERT_EQ("Darmstadt Algo", info.from_bike_station_);
    // ASSERT_EQ("Darmstadt HBF East", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
  {
    auto const& info = dep_infos[1];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(17, info.duration());
    // ASSERT_EQ("Darmstadt Mensa", info.from_bike_station_);
    // ASSERT_EQ("Darmstadt HBF East", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
  {
    auto const& info = dep_infos[2];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(17, info.duration());
    // ASSERT_EQ("Darmstadt Algo", info.from_bike_station_);
    // ASSERT_EQ("Darmstadt HBF West", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
  {
    auto const& info = dep_infos[3];
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(17, info.duration());
    // ASSERT_EQ("Darmstadt Mensa", info.from_bike_station_);
    // ASSERT_EQ("Darmstadt HBF West", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }

  ASSERT_EQ(4, arr_infos.size());
  {
    auto const& info = arr_infos[0];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(21, info.duration());
    // ASSERT_EQ("FFM HBF North", info.from_bike_station_);
    // ASSERT_EQ("FFM Westend 1", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
  {
    auto const& info = arr_infos[1];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(23, info.duration());
    // ASSERT_EQ("FFM HBF North", info.from_bike_station_);
    // ASSERT_EQ("FFM Westend 2", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
  {
    auto const& info = arr_infos[2];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(23, info.duration());
    // ASSERT_EQ("FFM HBF South", info.from_bike_station_);
    // ASSERT_EQ("FFM Westend 1", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
  {
    auto const& info = arr_infos[3];
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(25, info.duration());
    // ASSERT_EQ("FFM HBF South", info.from_bike_station_);
    // ASSERT_EQ("FFM Westend 2", info.to_bike_station_);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().from_);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().to_);
    ASSERT_EQ(4, info.availability_intervals_.front().rating_);
  }
}

void test_journey1(journey const& j, unsigned start_mumo_id) {
  ASSERT_EQ(4, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ(STATION_START, s.eva_no_);
    ASSERT_EQ(1421340180 /* Thu, 15 Jan 2015 16:43:00 GMT */,
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
    ASSERT_EQ(STATION_END, s.eva_no_);
    ASSERT_EQ(1421344080 /* Thu, 15 Jan 2015 17:48:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(3, j.transports_.size());
  ASSERT_TRUE(j.transports_[0].is_walk_);
  ASSERT_FALSE(j.transports_[1].is_walk_);
  ASSERT_TRUE(j.transports_[2].is_walk_);
  ASSERT_EQ(17, j.transports_[0].duration_);
  ASSERT_EQ(15, j.transports_[1].duration_);
  ASSERT_EQ(28, j.transports_[2].duration_);

  ASSERT_EQ(0, j.transports_[0].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[0].mumo_type_);
  ASSERT_EQ(start_mumo_id, j.transports_[0].mumo_id_);
  ASSERT_EQ(7, j.transports_[2].mumo_id_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[2].mumo_type_);
}

void test_journey2(journey const& j) {
  ASSERT_EQ(4, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ(STATION_START, s.eva_no_);
    ASSERT_EQ(1421338680 /* Thu, 15 Jan 2015 16:18:00 GMT */,
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
    ASSERT_EQ(STATION_END, s.eva_no_);
    ASSERT_EQ(1421342880 /* Thu, 15 Jan 2015 17:28:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(3, j.transports_.size());
  ASSERT_TRUE(j.transports_[0].is_walk_);
  ASSERT_FALSE(j.transports_[1].is_walk_);
  ASSERT_TRUE(j.transports_[2].is_walk_);

  ASSERT_EQ(17, j.transports_[0].duration_);
  ASSERT_EQ(5, j.transports_[1].duration_);
  ASSERT_EQ(28 + 15, j.transports_[2].duration_);

  ASSERT_EQ(0, j.transports_[0].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[0].mumo_type_);
  ASSERT_EQ(3, j.transports_[0].mumo_id_);
  ASSERT_EQ(7, j.transports_[2].mumo_id_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[2].mumo_type_);
}

/* Test a query interval larger than the availability interval */
TEST_F(reliability_bikesharing_routing, DISABLED_large_interval) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421336700, /* 15 Jan 2015 15:45:00 GMT */
                             1421342100 /* 15 Jan 2015 17:15:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true, false, false);
  auto res = call(req_msg);
  auto response = motis_content(ReliabilityRatingResponse, res);
  auto journeys = message_to_journeys(response->response());

  std::sort(journeys.begin(), journeys.end(),
            [](journey const& a, journey const& b) {
              return a.stops_.front().departure_.schedule_timestamp_ <
                     b.stops_.front().departure_.schedule_timestamp_;
            });

  ASSERT_EQ(2, journeys.size());
  test_journey2(journeys[0]);
  test_journey1(journeys[1], 2);

  auto infos = response->additional_infos();

  ASSERT_EQ(2, infos->size());
  {
    // id=3 2158078 49.875768,8.657800 49.872231,8.628254 17
    auto const& info = (*infos)[0];
    ASSERT_TRUE(info->at_departure()->valid());
    ASSERT_TRUE(std::abs(49.875768 - info->at_departure()->from()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.657800 - info->at_departure()->from()->lng()) <
                0.00001);
    ASSERT_TRUE(std::abs(49.872231 - info->at_departure()->to()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.628254 - info->at_departure()->to()->lng()) <
                0.00001);

    // id=7 4570367 50.107610,8.661005 50.128734,8.665265 28
    ASSERT_TRUE(info->at_arrival()->valid());
    ASSERT_TRUE(std::abs(50.107610 - info->at_arrival()->from()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.661005 - info->at_arrival()->from()->lng()) <
                0.00001);
    ASSERT_TRUE(std::abs(50.128734 - info->at_arrival()->to()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.665265 - info->at_arrival()->to()->lng()) < 0.00001);
  }
  {
    auto const& info = (*infos)[1];
    ASSERT_TRUE(info->at_departure()->valid());
    // id=2 2158078 49.875768,8.657800 49.872558,8.631700 17
    ASSERT_TRUE(std::abs(49.875768 - info->at_departure()->from()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.657800 - info->at_departure()->from()->lng()) <
                0.00001);
    ASSERT_TRUE(std::abs(49.872558 - info->at_departure()->to()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.631700 - info->at_departure()->to()->lng()) <
                0.00001);

    // id=7 4570367 50.107610,8.661005 50.128734,8.665265 28
    ASSERT_TRUE(info->at_arrival()->valid());
    ASSERT_TRUE(std::abs(50.107610 - info->at_arrival()->from()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.661005 - info->at_arrival()->from()->lng()) <
                0.00001);
    ASSERT_TRUE(std::abs(50.128734 - info->at_arrival()->to()->lat()) <
                0.00001);
    ASSERT_TRUE(std::abs(8.665265 - info->at_arrival()->to()->lng()) < 0.00001);
  }
}

TEST_F(reliability_bikesharing_routing, rating_request_small_query_interval) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421340180, /* 15 Jan 2015 16:43:00 GMT */
                             1421340180 /* 15 Jan 2015 16:43:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true, false, false);
  auto const journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());
  ASSERT_EQ(1, journeys.size());
  test_journey1(journeys[0], 3);
}

TEST_F(reliability_bikesharing_routing,
       waiting_at_destination_for_bikesharing) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421338680, /* 15 Jan 2015 16:18:00 GMT */
                             1421338680 /* 15 Jan 2015 16:18:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true, false, false);
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
                     .build_rating_request(true, false, false);
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
      ASSERT_EQ(STATION_END, s.eva_no_);
      ASSERT_EQ(1421342880 /* Thu, 15 Jan 2015 17:28:00 GMT */,
                s.arrival_.schedule_timestamp_);
    }

    ASSERT_EQ(2, j.transports_.size());
    ASSERT_FALSE(j.transports_[0].is_walk_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(5, j.transports_[0].duration_);

    ASSERT_EQ(3, j.transports_[1].mumo_id_);
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
      ASSERT_EQ(STATION_END, s.eva_no_);
      ASSERT_EQ(1421344080 /* Thu, 15 Jan 2015 17:48:00 GMT */,
                s.arrival_.schedule_timestamp_);
    }

    ASSERT_EQ(2, j.transports_.size());
    ASSERT_FALSE(j.transports_[0].is_walk_);
    ASSERT_TRUE(j.transports_[1].is_walk_);
    ASSERT_EQ(15, j.transports_[0].duration_);
    ASSERT_EQ(28, j.transports_[1].duration_);

    ASSERT_EQ(3, j.transports_[1].mumo_id_);
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
          .build_rating_request(true, false, false);
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
    ASSERT_EQ(STATION_END, s.eva_no_);
    ASSERT_EQ(1421342880 /* Thu, 15 Jan 2015 17:28:00 GMT */,
              s.arrival_.schedule_timestamp_);
  }

  ASSERT_EQ(2, j.transports_.size());
  ASSERT_FALSE(j.transports_[0].is_walk_);
  ASSERT_TRUE(j.transports_[1].is_walk_);
  ASSERT_EQ(5, j.transports_[0].duration_);

  ASSERT_EQ(3, j.transports_[1].mumo_id_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[1].mumo_type_);
}

TEST_F(reliability_bikesharing_routing, coordinates_to_station) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421340180, /* 15 Jan 2015 16:43:00 GMT */
                             1421340180 /* 15 Jan 2015 16:43:00 GMT */)
          .add_destination(schedule_bikesharing::FRANKFURT.name_,
                           schedule_bikesharing::FRANKFURT.eva_)
          .build_rating_request(true, false, false);
  auto journeys = message_to_journeys(
      motis_content(ReliabilityRatingResponse, call(req_msg))->response());
  ASSERT_EQ(1, journeys.size());

  auto const& j = journeys.front();

  ASSERT_EQ(3, j.stops_.size());
  {
    auto const& s = j.stops_[0];
    ASSERT_EQ(STATION_START, s.eva_no_);
    ASSERT_EQ(1421340180 /* Thu, 15 Jan 2015 16:43:00 GMT */,
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
  ASSERT_EQ(17, j.transports_[0].duration_);
  ASSERT_EQ(15, j.transports_[1].duration_);
  ASSERT_EQ(0, j.transports_[0].mumo_price_);
  ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
            j.transports_[0].mumo_type_);
}

TEST_F(reliability_bikesharing_routing, DISABLED_unreliable_bike) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421336700, /* 15 Jan 2015 15:45:00 GMT */
                             1421342100 /* 15 Jan 2015 17:15:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(false, true, false);
  auto res = call(req_msg);
  auto response = motis_content(ReliabilityRatingResponse, res);
  auto journeys = message_to_journeys(response->response());

  std::sort(journeys.begin(), journeys.end(),
            [](journey const& a, journey const& b) {
              return a.stops_.front().departure_.schedule_timestamp_ <
                     b.stops_.front().departure_.schedule_timestamp_;
            });

  ASSERT_EQ(2, journeys.size());

  {
    auto const& j = journeys[0];

    ASSERT_EQ(3, j.transports_.size());
    ASSERT_TRUE(j.transports_[0].is_walk_);
    ASSERT_FALSE(j.transports_[1].is_walk_);
    ASSERT_TRUE(j.transports_[2].is_walk_);

    ASSERT_EQ(17, j.transports_[0].duration_);
    ASSERT_EQ(5, j.transports_[1].duration_);
    ASSERT_EQ(28 /* no waiting for reliable bucket */,
              j.transports_[2].duration_);

    ASSERT_EQ(0, j.transports_[0].mumo_price_);
    ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
              j.transports_[0].mumo_type_);
    ASSERT_EQ(3, j.transports_[0].mumo_id_);
    ASSERT_EQ(7, j.transports_[2].mumo_id_);
    ASSERT_EQ(intermodal::to_str(intermodal::BIKESHARING),
              j.transports_[2].mumo_type_);
  }

  test_journey1(journeys[1], 2);

  ASSERT_EQ(2, response->additional_infos()->size());
  {
    auto i = (*response->additional_infos())[0];
    ASSERT_EQ(4, i->at_departure()->rating());
    ASSERT_EQ(0, i->at_arrival()->rating());
  }
  {
    auto i = (*response->additional_infos())[1];
    ASSERT_EQ(4, i->at_departure()->rating());
    ASSERT_EQ(4, i->at_arrival()->rating());
  }
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
