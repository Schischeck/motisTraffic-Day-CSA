#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/journey/message_to_journeys.h"

#include "motis/protocol/BikesharingRequest_generated.h"

#include "motis/reliability/intermodal/reliable_bikesharing.h"

#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/schedules/schedule_cg_bikesharing.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {

class reliability_bikesharing_cg : public test_motis_setup {
public:
  reliability_bikesharing_cg()
      : test_motis_setup(schedule_cg_bikesharing::PATH,
                         schedule_cg_bikesharing::DATE, false, true,
                         "modules/reliability/resources/nextbike_cg") {}
};

TEST_F(reliability_bikesharing_cg, cg) {
  ::motis::reliability::flatbuffers::request_builder b;
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg =
      b.add_intermodal_start(49.8776114, 8.6571044,
                             1454563620, /* 4 Feb 2016 05:27:00 GMT */
                             1454563620 /* 4 Feb 2016 05:27:00 GMT */)
          .add_intermodal_destination(50.1273104, 8.6669383)
          .build_reliable_search_request(1, true);
  auto res_msg = call(req_msg);
  ASSERT_NE(nullptr, res_msg);
  auto const cgs =
      motis_content(ReliableRoutingResponse, res_msg)->connection_graphs();
  ASSERT_EQ(1, cgs->size());

  auto const cg = (*cgs)[0];

  ASSERT_EQ(3, cg->stops()->size());
  {
    auto const stop = (*cg->stops())[0];
    ASSERT_EQ(0, stop->index());
    ASSERT_EQ(1, stop->alternatives()->size());
    ASSERT_EQ(0, (*stop->alternatives())[0]->journey());
    ASSERT_EQ(2, (*stop->alternatives())[0]->next_stop());
  }
  {
    auto const stop = (*cg->stops())[1];
    ASSERT_EQ(1, stop->index());
    ASSERT_EQ(0, stop->alternatives()->size());
  }
  {
    auto const stop = (*cg->stops())[2];
    ASSERT_EQ(2, stop->index());
    ASSERT_EQ(3, stop->alternatives()->size());
    {
      auto const ic = (*stop->alternatives())[0];
      ASSERT_EQ(1, ic->journey());
      ASSERT_EQ(1, ic->next_stop());
    }
    {
      auto const ic = (*stop->alternatives())[1];
      ASSERT_EQ(2, ic->journey());
      ASSERT_EQ(1, ic->next_stop());
    }
    {
      auto const ic = (*stop->alternatives())[2];
      ASSERT_EQ(3, ic->journey());
      ASSERT_EQ(1, ic->next_stop());
    }
  }

  ASSERT_EQ(4, cg->journeys()->size());
  {
    auto const j = (*cg->journeys())[0];
    ASSERT_EQ(3, j->stops()->size());
    ASSERT_EQ("START", (*j->stops())[0]->station()->id()->str());
    ASSERT_EQ("3333333", (*j->stops())[1]->station()->id()->str());
    ASSERT_EQ("2222222", (*j->stops())[2]->station()->id()->str());
    ASSERT_EQ(1454563620 /* Thu, 04 Feb 2016 05:27:00 GMT */,
              (*j->stops())[0]->departure()->time());
    ASSERT_EQ(1454565600 /* Thu, 04 Feb 2016 06:00:00 GMT */,
              (*j->stops())[1]->arrival()->time());
    ASSERT_EQ(1454565600 /* Thu, 04 Feb 2016 06:00:00 GMT */,
              (*j->stops())[1]->departure()->time());
    ASSERT_EQ(1454566200 /* Thu, 04 Feb 2016 06:10:00 GMT */,
              (*j->stops())[2]->arrival()->time());

    ASSERT_EQ(2, j->transports()->size());
    ASSERT_EQ(Move_Walk, (*j->transports())[0]->move_type());
    ASSERT_EQ(Move_Transport, (*j->transports())[1]->move_type());
    ASSERT_EQ(schedule_cg_bikesharing::RE_D_L,
              reinterpret_cast<Transport const*>((*j->transports())[1]->move())
                  ->train_nr());
  }
  {
    auto const j = (*cg->journeys())[1];
    ASSERT_EQ(3, j->stops()->size());
    ASSERT_EQ("2222222", (*j->stops())[0]->station()->id()->str());
    ASSERT_EQ("1111111", (*j->stops())[1]->station()->id()->str());
    ASSERT_EQ("END", (*j->stops())[2]->station()->id()->str());
    ASSERT_EQ(1454566500 /* Thu, 04 Feb 2016 06:15:00 GMT */,
              (*j->stops())[0]->departure()->time());
    ASSERT_EQ(1454567100 /* Thu, 04 Feb 2016 06:25:00 GMT */,
              (*j->stops())[1]->arrival()->time());
    ASSERT_EQ(1454567400 /* Thu, 04 Feb 2016 06:30:00 GMT */,
              (*j->stops())[1]->departure()->time());
    ASSERT_EQ(1454570100 /* Thu, 04 Feb 2016 07:15:00 GMT */,
              (*j->stops())[2]->arrival()->time());

    ASSERT_EQ(2, j->transports()->size());
    ASSERT_EQ(Move_Transport, (*j->transports())[0]->move_type());
    ASSERT_EQ(schedule_cg_bikesharing::RE_L_F,
              reinterpret_cast<Transport const*>((*j->transports())[0]->move())
                  ->train_nr());
    ASSERT_EQ(Move_Walk, (*j->transports())[1]->move_type());
  }
  {
    auto const j = (*cg->journeys())[2];
    ASSERT_EQ(3, j->stops()->size());
    ASSERT_EQ("2222222", (*j->stops())[0]->station()->id()->str());
    ASSERT_EQ("1111111", (*j->stops())[1]->station()->id()->str());
    ASSERT_EQ("END", (*j->stops())[2]->station()->id()->str());
    ASSERT_EQ(1454566560 /* Thu, 04 Feb 2016 06:16:00 GMT */,
              (*j->stops())[0]->departure()->time());
    ASSERT_EQ(1454567640 /* Thu, 04 Feb 2016 06:34:00 GMT */,
              (*j->stops())[1]->arrival()->time());
    ASSERT_EQ(1454567940 /* Thu, 04 Feb 2016 06:39:00 GMT */,
              (*j->stops())[1]->departure()->time());
    ASSERT_EQ(1454570640 /* Thu, 04 Feb 2016 07:24:00 GMT */,
              (*j->stops())[2]->arrival()->time());

    ASSERT_EQ(2, j->transports()->size());
    ASSERT_EQ(Move_Transport, (*j->transports())[0]->move_type());
    ASSERT_EQ(schedule_cg_bikesharing::S_L_F,
              reinterpret_cast<Transport const*>((*j->transports())[0]->move())
                  ->train_nr());
    ASSERT_EQ(Move_Walk, (*j->transports())[1]->move_type());
  }
  {
    auto const j = (*cg->journeys())[3];
    ASSERT_EQ(3, j->stops()->size());
    ASSERT_EQ("2222222", (*j->stops())[0]->station()->id()->str());
    ASSERT_EQ("1111111", (*j->stops())[1]->station()->id()->str());
    ASSERT_EQ("END", (*j->stops())[2]->station()->id()->str());
    ASSERT_EQ(1454566620 /* Thu, 04 Feb 2016 06:17:00 GMT */,
              (*j->stops())[0]->departure()->time());
    ASSERT_EQ(1454568000 /* Thu, 04 Feb 2016 06:40:00 GMT */,
              (*j->stops())[1]->arrival()->time());
    ASSERT_EQ(1454568300 /* Thu, 04 Feb 2016 06:45:00 GMT */,
              (*j->stops())[1]->departure()->time());
    ASSERT_EQ(1454571000 /* Thu, 04 Feb 2016 07:30:00 GMT */,
              (*j->stops())[2]->arrival()->time());

    ASSERT_EQ(2, j->transports()->size());
    ASSERT_EQ(Move_Transport, (*j->transports())[0]->move_type());
    ASSERT_EQ(schedule_cg_bikesharing::IC_L_F,
              reinterpret_cast<Transport const*>((*j->transports())[0]->move())
                  ->train_nr());
    ASSERT_EQ(Move_Walk, (*j->transports())[1]->move_type());
  }

  /* arrival distribution of the connection graph */
  probability_distribution exp_arr_dist;
  exp_arr_dist.init(
      {0.0592, 0.4884, 0.1776, 0.0148, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0192, 0.1584,
       0.0576, 0.0048, 0.0, 0.0, 0.0016, 0.0132, 0.0048, 0.0004},
      0);

  ASSERT_EQ(1454570040 /* Thu, 04 Feb 2016 07:14:00 GMT */,
            cg->arrival_distribution()->begin_time());
  ASSERT_EQ(exp_arr_dist.last_minute() + 1,
            cg->arrival_distribution()->distribution()->size());
  for (int i = 0; i <= exp_arr_dist.last_minute(); ++i) {
    ASSERT_TRUE(equal(exp_arr_dist.probability_equal(i),
                      (*cg->arrival_distribution()->distribution())[i]));
  }
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
