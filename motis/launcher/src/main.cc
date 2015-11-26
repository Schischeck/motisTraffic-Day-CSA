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

#include "motis/loader/loader.h"
#include "motis/loader/util.h"

#include "motis/module/module.h"
#include "motis/module/dispatcher.h"

#include "motis/guesser/guesser.h"
#include "motis/railviz/railviz.h"
#include "motis/reliability/reliability.h"
#include "motis/realtime/realtime.h"
#include "motis/ris/ris.h"
#include "motis/routing/routing.h"

#include "motis/launcher/ws_server.h"
#include "motis/launcher/dataset_settings.h"
#include "motis/launcher/listener_settings.h"
#include "motis/launcher/launcher_settings.h"

#include "motis/loader/parser_error.h"

#include "modules.h"

using namespace motis::launcher;
using namespace motis::module;
using namespace motis::logging;
using namespace motis;
using motis::loader::make_unique;

int main(int argc, char** argv) {
  message::init_parser();

  boost::asio::io_service ios, thread_pool;
  auto modules = build_modules();

  listener_settings listener_opt("0.0.0.0", "8080");
  dataset_settings dataset_opt("rohdaten", true, "TODAY", 2);
  launcher_settings launcher_opt(
      launcher_settings::SERVER,
      loader::transform_to_vec(
          begin(modules), end(modules),
          [](std::unique_ptr<motis::module::module> const& m) {
            return m->name();
          }));

  std::vector<conf::configuration*> confs = {&listener_opt, &dataset_opt,
                                             &launcher_opt};
  for (auto const& module : modules) {
    confs.push_back(module.get());
  }

  conf::options_parser parser(confs);
  parser.read_command_line_args(argc, argv);

  if (parser.help()) {
    std::cout << "\n\tMOTIS Webservice\n\n";
    parser.print_help(std::cout);
    return 0;
  } else if (parser.version()) {
    std::cout << "MOTIS Webservice\n";
    return 0;
  }

  parser.read_configuration_file();
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  auto schedule_interval = dataset_opt.interval();
  schedule_ptr sched;
  try {
    sched = loader::load_schedule(
        dataset_opt.dataset, dataset_opt.use_serialized,
        schedule_interval.first, schedule_interval.second);
  } catch (motis::loader::parser_error const& e) {
    std::cout << "unable to parse schedule\n";
    std::cout << e.filename << ":" << e.line_number << "\n";
    return 1;
  }

  ws_server server(ios);
  server.listen(listener_opt.host, listener_opt.port);

  dispatcher dispatcher(server, ios);

  namespace p = std::placeholders;
  send_fun send = std::bind(&dispatcher::send, &dispatcher, p::_1, p::_2);
  msg_handler dispatch =
      std::bind(&dispatcher::on_msg, &dispatcher, p::_1, p::_2, p::_3);

  motis::module::context c;
  c.schedule_ = sched.get();
  c.ios_ = &ios;
  c.thread_pool_ = &thread_pool;
  c.send_ = &send;
  c.dispatch_ = &dispatch;

  for (auto const& module : modules) {
    if (std::find(begin(launcher_opt.modules), end(launcher_opt.modules),
                  module->name()) == end(launcher_opt.modules)) {
      continue;
    }

    dispatcher.modules_.push_back(module.get());
    dispatcher.add_module(module.get());

    try {
      module->init_(&c);
    } catch (std::exception const& e) {
      LOG(emrg) << "module " << module->name()
                << ": unhandled init error: " << e.what();
      return 1;
    } catch (...) {
      LOG(emrg) << "module " << module->name()
                << "unhandled unknown init error";
      return 1;
    }
  }

  using net::http::server::shutdown_handler;
  shutdown_handler<ws_server> server_shutdown_handler(ios, server);

  boost::asio::io_service::work tp_work(thread_pool), ios_work(ios);
  std::vector<boost::thread> threads(8);

  auto run = [&]() {
    start:
      try {
        thread_pool.run();
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
    timer->async_wait([&server](boost::system::error_code) { server.stop(); });
  }

  LOG(info) << "listening on " << listener_opt.host << ":" << listener_opt.port;
  ios.run();
  thread_pool.stop();
  std::for_each(begin(threads), end(threads),
                [](boost::thread& t) { t.join(); });
  LOG(info) << "shutdown";
}
