#include "catch/catch.hpp"

#include <iostream>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/thread.hpp"

#include "motis/core/common/date_util.h"

#include "motis/loader/loader.h"

#include "motis/module/dispatcher.h"
#include "motis/module/server.h"

#include "motis/reliability/reliability.h"
#include "motis/routing/routing.h"

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

struct test_server : motis::module::server {
  void on_msg(msg_handler) override {}
  void on_open(sid_handler) override {}
  void on_close(sid_handler) override {}
  void send(msg_ptr const&, sid) override {}
};

msg_ptr to_flatbuffers_message(std::string const& from_name,
                               std::string const& from_eva,
                               std::string const& to_name,
                               std::string const& to_eva,
                               motis::time interval_begin,
                               motis::time interval_end) {
  /* convert routing::RoutingRequest to flatbuffers::Offset<RoutingRequest> */
  flatbuffers::FlatBufferBuilder b;
  std::vector<flatbuffers::Offset<routing::StationPathElement> >
      station_elements;
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(from_name), b.CreateString(from_eva)));
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(to_name), b.CreateString(to_eva)));
  routing::Interval interval(
      motis_to_unixtime(to_unix_time(2015, 9, 28), interval_begin),
      motis_to_unixtime(to_unix_time(2015, 9, 28), interval_end));
  b.Finish(CreateMessage(
      b, MsgContent_RoutingRequest,
      routing::CreateRoutingRequest(b, &interval, routing::Type::Type_PreTrip,
                                    routing::Direction::Direction_Forward,
                                    b.CreateVector(station_elements))
          .Union()));
  return make_msg(b);
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
  boost::asio::io_service ios;

  test_server t;
  motis::module::dispatcher dispatcher(t, ios);
  namespace p = std::placeholders;
  msg_handler dispatch =
      std::bind(&dispatcher::on_msg, &dispatcher, p::_1, p::_2, p::_3);

  motis::module::context c;
  c.schedule_ = schedule.get();
  c.thread_pool_ = &ios;
  c.ios_ = &ios;
  c.dispatch_ = &dispatch;

  std::vector<std::unique_ptr<motis::module::module> > modules;
  modules.emplace_back(new routing::routing());
  modules.emplace_back(new reliability::reliability());
  for (auto const& module : modules) {
    dispatcher.modules_.push_back(module.get());
    dispatcher.add_module(module.get());
    module->init_(&c);
  }

  auto msg = to_flatbuffers_message(
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

  dispatcher.on_msg(msg, 0, test_cb);
  ios.run();
}
