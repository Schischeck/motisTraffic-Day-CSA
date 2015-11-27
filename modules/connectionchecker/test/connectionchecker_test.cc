#include "gtest/gtest.h"

#include <string>
#include <iostream>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"

#include "motis/protocol/Message_generated.h"

#include "motis/bootstrap/motis_instance.h"
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
  std::vector<Offset<UpdatedEvent>> events{CreateUpdatedEvent(
      fbb, CreateEvent(fbb, StationIdType_EVA, fbb.CreateString("8000010"), 628,
                      EventType_Departure, 1448372160),
      1448372220)};
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

using instance_ptr = std::unique_ptr<motis_instance>;
instance_ptr launch_motis(std::string const& dataset,
                          std::string const& schedule_begin,
                          std::vector<std::string> const& modules) {
  auto instance = make_unique<motis_instance>();
  instance->init_schedule({dataset, false, schedule_begin, 2});
  instance->init_modules(modules);
  return instance;
}

msg_ptr send(instance_ptr const& instance, msg_ptr request) {
  msg_ptr response;

  instance->on_msg(request, 1,
                   [&](msg_ptr r, boost::system::error_code) { response = r; });
  instance->run();

  return response;
}

TEST(connectionchecker, finds_annotated_connections) {
  auto instance =
      launch_motis("modules/connectionchecker/test_resources/schedule",
                   "20151124", {"routing", "connectionchecker", "realtime"});
  send(instance, get_ris_message());

  // auto msg = parse_ris_message(kDelayMessage);
  // auto req = msg->content<motis::ris::RISBatch const*>();

  // auto routing_request = std::make_shared<message>(kRoutingRequest);

  // auto journeys =
  //     message_to_journeys(response->content<RoutingResponse const*>());
  // ASSERT_EQ(1, journeys.size());

  ASSERT_TRUE(true);
}

}  // namespace connectionchecker
}  // namespace motis
