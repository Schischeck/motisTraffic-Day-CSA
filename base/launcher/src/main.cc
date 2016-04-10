#include <iostream>
#include <memory>
#include <thread>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/filesystem.hpp"
#include "boost/thread.hpp"

#include "net/http/server/shutdown_handler.hpp"

#include "conf/options_parser.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/util.h"

#include "motis/loader/util.h"

#include "motis/bootstrap/dataset_settings.h"
#include "motis/bootstrap/motis_instance.h"

#include "motis/launcher/batch_mode.h"
#include "motis/launcher/http_server.h"
#include "motis/launcher/launcher_settings.h"
#include "motis/launcher/listener_settings.h"
#include "motis/launcher/socket_server.h"
#include "motis/launcher/ws_server.h"

#include "modules.h"
#include "version.h"

using namespace motis::bootstrap;
using namespace motis::launcher;
using namespace motis::module;
using namespace motis::logging;
using namespace motis;

using net::http::server::shutdown_handler;

template <typename T>
using shutd_hdr_ptr = std::unique_ptr<shutdown_handler<T>>;

int main(int argc, char** argv) {
  motis_instance instance;
  auto& ios = instance.ios_;
  ws_server websocket(ios, instance);
  http_server http(ios, instance);
  socket_server tcp(ios, instance);

  listener_settings listener_opt(true, false, false, "0.0.0.0", "8080",
                                 "0.0.0.0", "8081", "0.0.0.0", "7000", "");
  dataset_settings dataset_opt("rohdaten", true, true, true, true, "TODAY", 2);
  launcher_settings launcher_opt(
      launcher_settings::motis_mode_t::SERVER,
      loader::transform_to_vec(
          begin(instance.modules_), end(instance.modules_),
          [](std::unique_ptr<motis::module::module> const& m) {
            return m->name();
          }),
      "queries.txt", "responses.txt", std::thread::hardware_concurrency());

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

  net::http::server::io_service_shutdown shutd(ios);
  shutdown_handler<net::http::server::io_service_shutdown> shutdown(ios, shutd);

  shutd_hdr_ptr<ws_server> websocket_shutdown_handler;
  shutd_hdr_ptr<http_server> http_shutdown_handler;
  shutd_hdr_ptr<socket_server> tcp_shutdown_handler;
  try {
    instance.init_schedule(dataset_opt);
    instance.init_modules(launcher_opt.modules);

    if (listener_opt.listen_ws) {
      websocket.set_api_key(listener_opt.api_key);
      websocket.listen(listener_opt.ws_host, listener_opt.ws_port);
      websocket_shutdown_handler =
          make_unique<shutdown_handler<ws_server>>(ios, websocket);
    }

    if (listener_opt.listen_http) {
      http.listen(listener_opt.http_host, listener_opt.http_port);
      http_shutdown_handler =
          make_unique<shutdown_handler<http_server>>(ios, http);
    }

    if (listener_opt.listen_tcp) {
      tcp.listen(listener_opt.tcp_host, listener_opt.tcp_port);
      tcp_shutdown_handler =
          make_unique<shutdown_handler<socket_server>>(ios, tcp);
    }
  } catch (std::exception const& e) {
    std::cout << "\ninitialization error: " << e.what() << "\n";
    return 1;
  }

  boost::asio::io_service::work ios_work(ios);
  std::vector<boost::thread> threads(launcher_opt.num_threads);

  auto run = [&ios]() {
    start:
      try {
        ios.run();
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
  if (launcher_opt.mode == launcher_settings::motis_mode_t::TEST) {
    timer = make_unique<boost::asio::deadline_timer>(
        ios, boost::posix_time::seconds(1));
    timer->async_wait([&ios](boost::system::error_code) { ios.stop(); });
  } else if (launcher_opt.mode == launcher_settings::motis_mode_t::BATCH) {
    inject_queries(ios, instance, launcher_opt.batch_input_file,
                   launcher_opt.batch_output_file);
  }

  std::for_each(begin(threads), end(threads),
                [](boost::thread& t) { t.join(); });
  LOG(info) << "shutdown";
}
