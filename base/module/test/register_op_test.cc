#include "gtest/gtest.h"

#include "boost/asio/io_service.hpp"

#include "motis/module/dispatcher.h"
#include "motis/module/message.h"
#include "motis/module/motis_call.h"

using namespace motis;
using namespace motis::module;

auto query = R"({
  "destination": {
    "type": "Module",
    "target": "/routing"
  },
  "content_type": "RoutingRequest",
  "content": {
    "interval": {
      "begin": 1444896228,
      "end": 1444899228
    },
    "type": "PreTrip",
    "direction": "Forward",
    "path": [
      { "eva_nr": "8000096", "name": "" },
      { "eva_nr": "8000105", "name": "" }
    ],
    "additional_edges": []
  }
})";

auto guess = [](msg_ptr const&) {
  MessageCreator b;
  b.CreateAndFinish(
      MsgContent_StationGuesserResponse,
      motis::guesser::CreateStationGuesserResponse(
          b, b.CreateVector(
                 std::vector<flatbuffers::Offset<motis::guesser::Station>>(
                     {motis::guesser::CreateStation(
                         b, b.CreateString("Darmstadt Hbf"),
                         b.CreateString("8600068"), 0, 0)})))
          .Union());
  return make_msg(b);
};

auto route = [](msg_ptr const&) -> msg_ptr {
  MessageCreator b;
  b.CreateAndFinish(
      MsgContent_StationGuesserRequest,
      motis::guesser::CreateStationGuesserRequest(b, 1, b.CreateString("test"))
          .Union(),
      "/guesser");
  auto station = motis_call(make_msg(b));

  b.CreateAndFinish(
      MsgContent_RoutingResponse,
      motis::routing::CreateRoutingResponse(
          b, 0,
          b.CreateVector(
              std::vector<flatbuffers::Offset<motis::routing::Connection>>()))
          .Union());
  return make_msg(b);
};

TEST(module_op, launch) {
  boost::asio::io_service ios;
  registry r;
  dispatcher d(ios, r);

  r.register_op("/guesser", guess);
  r.register_op("/routing", route);

  msg_ptr result;

  d.scheduler_.enqueue(
      ctx_data(&d, std::make_shared<snapshot>(),
               std::make_shared<std::map<std::string, std::string>>()),
      [&]() { result = motis_call(make_msg(query))->val(); },
      ctx::op_id(CTX_LOCATION));

  ios.run();

  ASSERT_TRUE(result);
  ASSERT_EQ(MsgContent_RoutingResponse, result->content_type());
}
