#include "catch/catch.hpp"

#include <iostream>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/loader/loader.h"

#include "motis/module/dispatcher.h"

#include "motis/routing/routing.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

using namespace motis;
using namespace motis::module;

namespace schedule2 {
struct station {
  std::string name;
  std::string eva;
};
station const ERLANGEN = {"Erlangen", "0953067"};
station const KASSEL = {"Kassel", "6380201"};
station const STUTTGART = {"Stuttgart", "7309882"};
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const ICE_E_K = 7;  // 12:45 --> 14:15
}

void foo(routing::Connection const* c) {
  std::cout << std::endl << std::endl;
  for (auto it = c->stops()->begin(); it != c->stops()->end(); ++it) {
    std::cout << "Stop " << it->name()->str() << std::endl;
  }
}

TEST_CASE("request", "[reliability]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  reliability::system_tools::setup setup(schedule.get());

  auto msg = reliability::flatbuffers_tools::to_flatbuffers_message(
      schedule2::STUTTGART.name, schedule2::STUTTGART.eva,
      schedule2::KASSEL.name, schedule2::KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35));

  auto test_cb = [=](motis::module::msg_ptr msg, boost::system::error_code e) {
    // std::cout << "--------------ok\n" << msg->to_json() << std::endl;
    auto response = msg->content<routing::RoutingResponse const*>();
    for (auto it = response->connections()->begin();
         it != response->connections()->end(); ++it) {
      foo(*it);
    }
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
