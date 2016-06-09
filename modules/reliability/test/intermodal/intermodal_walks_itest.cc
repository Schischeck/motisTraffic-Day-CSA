#include "gtest/gtest.h"

#include "motis/core/common/constants.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/schedules/schedule_bikesharing.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {

class reliability_intermodal_walks : public test_motis_setup {
public:
  reliability_intermodal_walks()
      : test_motis_setup(schedule_bikesharing::PATH, schedule_bikesharing::DATE,
                         false, true,
                         "modules/reliability/resources/nextbike_routing") {}
};

TEST_F(reliability_intermodal_walks, walk_at_begin) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.872686, 8.632742,
                             1421336700, /* 15 Jan 2015 15:45:00 GMT */
                             1421342100 /* 15 Jan 2015 17:15:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_rating_request(true, false, true);
  auto const res_msg = call(req_msg);
  auto const res = motis_content(ReliabilityRatingResponse, res_msg);
  auto journeys = message_to_journeys(res->response());

  std::sort(journeys.begin(), journeys.end(),
            [](journey const& a, journey const& b) {
              return a.stops_.front().departure_.schedule_timestamp_ <
                     b.stops_.front().departure_.schedule_timestamp_;
            });

  ASSERT_EQ(2, journeys.size());

  for (auto const& j : journeys) {
    ASSERT_EQ(3, j.transports_.size());
    ASSERT_TRUE(j.transports_[0].is_walk_);
    ASSERT_EQ("Walk", j.transports_[0].mumo_type_);
    ASSERT_EQ(3, j.transports_[0].duration_);
    ASSERT_EQ("Bikesharing", j.transports_[2].mumo_type_);
  }

  ASSERT_EQ(2, res->additional_infos()->size());
  for (auto const& info : *res->additional_infos()) {
    ASSERT_FALSE(info->at_departure()->valid());
    ASSERT_TRUE(info->at_arrival()->valid());
  }
}

TEST_F(reliability_intermodal_walks, walk_at_end) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1421336700, /* 15 Jan 2015 15:45:00 GMT */
                             1421342100 /* 15 Jan 2015 17:15:00 GMT */)
          .add_intermodal_destination(50.108290, 8.664139)
          .build_rating_request(true, false, true);
  auto const res_msg = call(req_msg);
  auto const res = motis_content(ReliabilityRatingResponse, res_msg);
  auto journeys = message_to_journeys(res->response());

  std::sort(journeys.begin(), journeys.end(),
            [](journey const& a, journey const& b) {
              return a.stops_.front().departure_.schedule_timestamp_ <
                     b.stops_.front().departure_.schedule_timestamp_;
            });

  ASSERT_EQ(1, journeys.size());

  auto const& j = journeys[0];
  ASSERT_EQ(3, j.transports_.size());
  ASSERT_TRUE(j.transports_[0].is_walk_);
  ASSERT_EQ("Bikesharing", j.transports_[0].mumo_type_);
  ASSERT_EQ("Walk", j.transports_[2].mumo_type_);
  ASSERT_EQ(4, j.transports_[2].duration_);

  ASSERT_EQ(1, res->additional_infos()->size());
  auto const info = (*res->additional_infos())[0];
  ASSERT_TRUE(info->at_departure()->valid());
  ASSERT_FALSE(info->at_arrival()->valid());
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
