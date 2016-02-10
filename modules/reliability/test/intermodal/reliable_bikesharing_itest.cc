#include "gtest/gtest.h"

#include <iostream>

#include "motis/protocol/BikesharingRequest_generated.h"

#include "motis/reliability/intermodal/reliable_bikesharing.h"

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

TEST_F(reliability_bikesharing, test_request) {
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg = to_bikesharing_request(
      49.8776114, 8.6571044, 50.1273104, 8.6669383,
      1454602500, /* Thu, 04 Feb 2016 16:15:00 GMT */
      1454606100, /* Thu, 04 Feb 2016 17:15:00 GMT */
      motis::bikesharing::AvailabilityAggregator_Average);
  auto msg = test::send(motis_instance_, req_msg);
  ASSERT_NE(nullptr, msg);
  auto response =
      msg->content<::motis::bikesharing::BikesharingResponse const*>();

  ASSERT_EQ(4, response->departure_edges()->size());
  ASSERT_EQ(4, response->arrival_edges()->size());
}

void test_bikesharing_infos(std::pair<std::vector<bikesharing_info>,
                                      std::vector<bikesharing_info>> infos,
                            std::shared_ptr<bool> test_cb_called) {
  *test_cb_called = true;

  auto sort = [](std::vector<bikesharing_info>& infos) {
    std::sort(infos.begin(), infos.end(), [](bikesharing_info const& a,
                                             bikesharing_info const& b) {
      return a.station_eva_ < b.station_eva_ ||
             (a.station_eva_ == b.station_eva_ && a.duration_ < b.duration_);
    });
  };
  sort(infos.first);
  sort(infos.second);

  ASSERT_EQ(4, infos.first.size());
  {
    auto const& info = infos.first.front();
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(34, info.duration_);
    ASSERT_EQ("Darmstadt Algo", info.bikesharing_stations_.first);
    ASSERT_EQ("Darmstadt HBF East", info.bikesharing_stations_.second);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.first.back();
    ASSERT_EQ("8000068", info.station_eva_);
    ASSERT_EQ(40, info.duration_);
    ASSERT_EQ("Darmstadt Mensa", info.bikesharing_stations_.first);
    ASSERT_EQ("Darmstadt HBF West", info.bikesharing_stations_.second);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }

  ASSERT_EQ(4, infos.second.size());
  {
    auto const& info = infos.second.front();
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(44, info.duration_);
    ASSERT_EQ("FFM HBF North", info.bikesharing_stations_.first);
    ASSERT_EQ("FFM Westend 1", info.bikesharing_stations_.second);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
  {
    auto const& info = infos.second.back();
    ASSERT_EQ("8000105", info.station_eva_);
    ASSERT_EQ(50, info.duration_);
    ASSERT_EQ("FFM HBF South", info.bikesharing_stations_.first);
    ASSERT_EQ("FFM Westend 2", info.bikesharing_stations_.second);
    ASSERT_EQ(1, info.availability_intervals_.size());
    ASSERT_EQ(1454601600, info.availability_intervals_.front().first);
    ASSERT_EQ(1454605200, info.availability_intervals_.front().second);
  }
}

TEST_F(reliability_bikesharing, retrieve_bikesharing_infos) {
  auto test_cb_called = std::make_shared<bool>(false);
  auto aggregator = std::make_shared<average_aggregator>(4);
  retrieve_bikesharing_infos(
      to_bikesharing_request(49.8776114, 8.6571044, 50.1273104, 8.6669383,
                             1454602500, /* Thu, 04 Feb 2016 16:15:00 GMT */
                             1454606100, /* Thu, 04 Feb 2016 17:15:00 GMT */
                             aggregator->get_aggregator()),
      aggregator, get_reliability_module(), 0,
      std::bind(&test_bikesharing_infos, std::placeholders::_1,
                test_cb_called));
  motis_instance_->run();
  ASSERT_TRUE(*test_cb_called);
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
