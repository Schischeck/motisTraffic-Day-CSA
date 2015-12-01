#include "gtest/gtest.h"

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"

#include "motis/protocol/Message_generated.h"

#include "motis/bootstrap/motis_instance_helper.h"
#include "motis/core/common/util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/message.h"

using namespace flatbuffers;
using namespace motis::bootstrap;
using namespace motis::module;
using namespace motis::ris;
using namespace motis::routing;

namespace motis {
namespace connectionchecker {

constexpr char const* kRoutingRequest = R""(
{
  "content_type": "RoutingRequest",
  "content": {
    "interval": {
      "begin": 1448368200,  // 2015-11-24 13:30:00 GMT+0100
      "end": 1448368200  // 2015-11-24 13:30:00 GMT+0100
    },
    "type": "OnTrip",
    "direction": "Forward",
    "path": [
      { "eva_nr": "8000260", "name": "" }, // Würzburg
      { "eva_nr": "8000105", "name": "" }  // TODO
    ]
  }
}
)"";

msg_ptr get_ris_message() {
  FlatBufferBuilder fbb;
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          StationIdType_EVA,
          fbb.CreateString("8000010"),  // Aschaffenburg
          628,
          EventType_Departure,
          1448372160  // 2015-11-24 14:36:00 GMT+0100
        ),
      1448372220  // 2015-11-24 14:37:00 GMT+0100
    )};
  // clang-format on
  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, DelayType_Is, fbb.CreateVector(events)).Union()));

  MessageCreator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.CreateAndFinish(MsgContent_RISBatch,
                     CreateRISBatch(mc, mc.CreateVector(messages)).Union());
  return make_msg(mc);
}

TEST(connectionchecker, finds_annotated_connections) {
  auto instance =
      launch_motis("modules/connectionchecker/test_resources/schedule",
                   "20151124", {"routing", "connectionchecker", "realtime"});
  send(instance, get_ris_message());

  auto request = std::make_shared<message>(kRoutingRequest);
  auto response = send(instance, request);
  auto journeys =
      message_to_journeys(response->content<RoutingResponse const*>());
  ASSERT_EQ(1, journeys.size());
  auto j = journeys[0];

  auto s0 = j.stops[0];
  EXPECT_EQ(std::string("8000260"), s0.eva_no);
  EXPECT_EQ(0, s0.arrival.timestamp);
  EXPECT_EQ(0, s0.arrival.schedule_timestamp);
  // 2015-11-24 13:55:00 GMT+0100
  EXPECT_EQ(1448369700, s0.departure.timestamp);
  EXPECT_EQ(1448369700, s0.departure.schedule_timestamp);
}

}  // namespace connectionchecker
}  // namespace motis
