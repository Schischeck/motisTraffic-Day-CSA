#include <iostream>
#include <memory>

#include "boost/filesystem.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/thread.hpp"

#include "net/http/server/shutdown_handler.hpp"

#include "conf/options_parser.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/util.h"

#include "motis/loader/util.h"

#include "motis/bootstrap/dataset_settings.h"
#include "motis/bootstrap/motis_instance.h"

#include "motis/launcher/ws_server.h"
#include "motis/launcher/http_server.h"
#include "motis/launcher/listener_settings.h"
#include "motis/launcher/launcher_settings.h"

#include "version.h"
#include "modules.h"

using namespace motis::bootstrap;
using namespace motis::launcher;
using namespace motis::module;
using namespace motis::logging;
using namespace motis;

int main(int argc, char** argv) {
  message::init_parser();

  boost::asio::io_service ios;
  motis_instance instance(&ios);
  ws_server websocket(ios, instance);
  http_server http(ios, instance);

  listener_settings listener_opt(true, false, "0.0.0.0", "8080", "0.0.0.0",
                                 "8081", "");
  dataset_settings dataset_opt("rohdaten", true, true, false, "TODAY", 2);
  launcher_settings launcher_opt(
      launcher_settings::SERVER,
      loader::transform_to_vec(
          begin(instance.modules_), end(instance.modules_),
          [](std::unique_ptr<motis::module::module> const& m) {
            return m->name();
          }));

  std::vector<conf::configuration*> confs = {&listener_opt, &dataset_opt,
                                             &launcher_opt};
  for (auto const& module : instance.modules()) {
    confs.push_back(module);
  }

  try {
    conf::options_parser parser(confs);
    parser.read_command_line_args(argc, argv, false);

    if (parser.help()) {
      std::cout << "\n\tMOTIS v" << short_version() << "\n\n";
      parser.print_help(std::cout);
      return 0;
    } else if (parser.version()) {
      std::cout << "MOTIS v" << long_version() << "\n";
      return 0;
    }

    parser.read_configuration_file(false);
    parser.print_used(std::cout);
  } catch (std::exception const& e) {
    std::cout << "options error: " << e.what() << "\n";
    return 1;
  }

  try {
    instance.set_send_fun([&websocket](msg_ptr msg, sid session) {
      websocket.send(msg, session);
    });
    instance.init_schedule(dataset_opt);
    instance.init_modules(launcher_opt.modules);

    if (listener_opt.listen_ws) {
      websocket.set_api_key(listener_opt.api_key);
      websocket.listen(listener_opt.ws_host, listener_opt.ws_port);
    }

    if (listener_opt.listen_http) {
      http.listen(listener_opt.http_host, listener_opt.http_port);
    }
  } catch (std::exception const& e) {
    std::cout << "initialization error: " << e.what() << "\n";
    return 1;
  }

  using net::http::server::shutdown_handler;
  shutdown_handler<ws_server> server_shutdown_handler(ios, websocket);

  boost::asio::io_service::work tp_work(instance.thread_pool_), ios_work(ios);
  std::vector<boost::thread> threads(8);

  auto run = [&]() {
    start:
      try {
        instance.run();
      } catch (std::exception const& e) {
        LOG(emrg) << "unhandled error: " << e.what();
        goto start;
      } catch (...) {
        LOG(emrg) << "unhandled unknown error";
        goto start;
      }
  };

  for (unsigned i = 0; i < threads.size(); ++i) {
    threads[i] = boost::thread(run);
  }

  std::unique_ptr<boost::asio::deadline_timer> timer;
  if (launcher_opt.mode == launcher_settings::TEST) {
    timer = make_unique<boost::asio::deadline_timer>(
        ios, boost::posix_time::seconds(1));
    timer->async_wait(
        [&websocket](boost::system::error_code) { websocket.stop(); });
  }

  std::function<void()> run_server = [&ios, &run_server]() {
    try {
      ios.run();
    } catch (...) {
      run_server();
    }
  };
  run_server();
  instance.thread_pool_.stop();
  std::for_each(begin(threads), end(threads),
                [](boost::thread& t) { t.join(); });
  LOG(info) << "shutdown";
}
