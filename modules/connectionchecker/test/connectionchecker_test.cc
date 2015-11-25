#include "gtest/gtest.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"

#include "motis/protocol/Message_generated.h"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/message.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::ris;
using namespace motis::routing;

namespace motis {
namespace connectionchecker {

constexpr char const* kDelayMessage = R""(
{
  "message_type": "DelayMessage",
  "message": {
    "content": {
      "type": "Is",
      "events": [{
        "base": {
          "stationIdType": "EVA",
          "stationId": "8000010",
          "trainIndex": ""
          "type": "Departure"
          "scheduledTime": "1448372160"
        },
        "updatedTime": "1448372220"
      }]
    }
  }
}
)"";

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

msg_ptr parse_ris_message(char const* raw) {
  Parser parser;
  parser.Parse(raw);

  MessageCreator b;
  std::vector<Offset<MessageHolder>> message_offsets;
  message_offsets.push_back(
      CreateMessageHolder(b, b.CreateVector(parser.builder_.GetBufferPointer(),
                                            parser.builder_.GetSize())));
  b.CreateAndFinish(MsgContent_RISBatch,
                    CreateRISBatch(b, b.CreateVector(message_offsets)).Union());
  return make_msg(b);
}

TEST(connectionchecker, finds_annotated_connections) {
  auto delay_message = parse_ris_message(kDelayMessage);
  auto routing_request = std::make_shared<message>(kRoutingRequest);

  msg_ptr response;

  auto journeys = message_to_journeys(response->content<RoutingResponse const*>());
  ASSERT_EQ(1, journeys.size());


  ASSERT_TRUE(true);
}

}  // namespace connectionchecker
}  // namespace motis
