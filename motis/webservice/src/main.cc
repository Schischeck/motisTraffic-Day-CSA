#include <iostream>

#include "net/http/server/shutdown_handler.hpp"

#include "motis/loader/Loader.h"

#include "motis/module/module.h"
#include "motis/module/dispatcher.h"

#include "motis/webservice/ws_server.h"

#include "motis/railviz/railviz.h"

using namespace motis::webservice;
using namespace motis::module;
using namespace td;

int main() {
  auto schedule = loadSchedule("../schedule/test");

  boost::asio::io_service ios;

  ws_server server(ios);
  server.listen({"0.0.0.0", "9002"});

  dispatcher<ws_server> dispatcher(server);
  dispatcher.add_module(load_module(schedule.get()).release());

  using net::http::server::shutdown_handler;
  shutdown_handler<ws_server> server_shutdown_handler(ios, server);

  ios.run();
  std::cout << "shutdown\n";
}