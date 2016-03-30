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
                 std::vector<flatbuffers::Offset<motis::guesser::Station>>()))
          .Union());
  return make_msg(b);
};

auto route = [](msg_ptr const&) {
  MessageCreator b;
  b.CreateAndFinish(
      MsgContent_StationGuesserRequest,
      motis::guesser::CreateStationGuesserRequest(b, 1, b.CreateString("test"))
          .Union(),
      "/guesser");
  auto station = motis_call(make_msg(b));
  printf("%s\n", station->val()->to_json().c_str());

  b.CreateAndFinish(
      MsgContent_RoutingResponse,
      motis::routing::CreateRoutingResponse(
          b, 0,
          b.CreateVector(
              std::vector<flatbuffers::Offset<motis::routing::Connection>>()))
          .Union());
  return make_msg(b);
};

auto input = []() {
  auto response = motis_call(make_msg(query));
  printf("%s\n", response->val()->to_json().c_str());
};

TEST(module_op, launch) {
  boost::asio::io_service ios;
  registry r;
  dispatcher d(ios, r);

  r.operations_["/guesser"].fn_ = guess;
  r.operations_["/routing"].fn_ = route;

  d.scheduler_.enqueue(
      ctx_data(&d, std::make_shared<snapshot>(),
               std::make_shared<std::map<std::string, std::string>>()),
      input, ctx::op_id(CTX_LOCATION));

  ios.run();
}
