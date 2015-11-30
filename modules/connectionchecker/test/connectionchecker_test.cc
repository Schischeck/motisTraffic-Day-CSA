
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
      "begin": 1448371800,
      "end": 1448371800
    },
    "type": "OnTrip",
    "direction": "Forward",
    "path": [
      { "eva_nr": "8000260", "name": "" },
      { "eva_nr": "8000105", "name": "" }
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
          fbb.CreateString("8000010"),
          628,
          EventType_Departure,
          1448372160
        ),
      1448372220
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
}

}  // namespace connectionchecker
}  // namespace motis
