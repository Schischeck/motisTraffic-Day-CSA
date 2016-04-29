#include "gtest/gtest.h"

#include "motis/core/access/time_access.h"
#include "motis/module/message.h"

#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace flatbuffers;
using namespace motis;
using namespace motis::module;
using namespace motis::routing;
using namespace motis::test;
using namespace motis::test::schedule::simple_realtime;

TEST(routing_ontrip_train, stay_in_train) {
  auto motis = launch_motis(kSchedulePath, kScheduleDate, {"routing"});
  auto const& sched = *motis->schedule_;

  message_creator fbb;
  fbb.create_and_finish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(
          fbb, Start_OntripTrainStart,
          CreateOntripTrainStart(
              fbb,
              CreateTripId(fbb, fbb.CreateString("8000096"), 2292,
                           unix_time(sched, 1305), fbb.CreateString("8000105"),
                           unix_time(sched, 1440), EventType_Departure,
                           fbb.CreateString("381")),
              CreateInputStation(fbb, fbb.CreateString("8000068"),
                                 fbb.CreateString("")),
              unix_time(sched, 1422))
              .Union(),
          CreateInputStation(fbb, fbb.CreateString("8000001"),
                             fbb.CreateString("")),
          fbb.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()),
          fbb.CreateVector(std::vector<Offset<Via>>()))
          .Union(),
      "/routing");
  EXPECT_EQ(1, motis_content(RoutingResponse, call(motis, make_msg(fbb)))
                   ->connections()
                   ->size());
}