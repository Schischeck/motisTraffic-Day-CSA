#include "gtest/gtest.h"

#include "boost/asio/io_service.hpp"

#include "motis/module/context/motis_call.h"
#include "motis/module/controller.h"
#include "motis/module/dispatcher.h"
#include "motis/module/message.h"

using namespace motis;
using namespace motis::module;
using namespace motis::routing;

auto query = R"({
  "destination": {
    "type": "Module",
    "target": "/routing"
  },
  "content_type": "RoutingRequest",
  "content": {
    "start_type": "PretripStart",
    "start": {
      "station": {
        "name": "",
        "id": "8000096"
      },
      "interval": {
        "begin": 1444896228,
        "end": 1444899228
      }
    },
    "destination": {
      "name": "",
      "id": "8000105"
    },
    "additional_edges": [],
    "via": []
  }
})";

auto guess = [](msg_ptr const&) {
  message_creator b;
  auto const pos = Position(0, 0);
  b.create_and_finish(
      MsgContent_StationGuesserResponse,
      motis::guesser::CreateStationGuesserResponse(
          b, b.CreateVector(std::vector<flatbuffers::Offset<Station>>(
                 {CreateStation(b, b.CreateString("Darmstadt Hbf"),
                                b.CreateString("8600068"), &pos)})))
          .Union());
  return make_msg(b);
};

auto route = [](msg_ptr const&) -> msg_ptr {
  message_creator b;
  b.create_and_finish(
      MsgContent_StationGuesserRequest,
      motis::guesser::CreateStationGuesserRequest(b, 1, b.CreateString("test"))
          .Union(),
      "/guesser");
  auto station = motis_call(make_msg(b));

  routing::Statistics s(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  b.create_and_finish(
      MsgContent_RoutingResponse,
      motis::routing::CreateRoutingResponse(
          b, &s, b.CreateVector(std::vector<flatbuffers::Offset<Connection>>()))
          .Union());
  return make_msg(b);
};

TEST(module_op, launch) {
  controller c;
  c.register_op("/guesser", guess);
  c.register_op("/routing", route);

  auto result = c.run([]() { return motis_call(make_msg(query))->val(); });

  ASSERT_TRUE(result);
  motis_content(RoutingResponse, result);
}
