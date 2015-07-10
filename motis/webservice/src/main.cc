#include <iostream>

#include "boost/filesystem.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/signal_set.hpp"

#include "net/http/server/shutdown_handler.hpp"

#include "conf/options_parser.h"

#include "motis/loader/loader.h"

#include "motis/module/module.h"
#include "motis/module/dispatcher.h"

#ifndef MOTIS_STATIC_MODULES
#include "motis/module/dynamic_module.h"
#include "motis/module/dynamic_module_loader.h"
#else
#include "motis/railviz/railviz.h"
#include "motis/guesser/guesser.h"
#include "motis/routing/routing.h"
#include "motis/reliability/reliability.h"
#endif

#include "motis/webservice/ws_server.h"
#include "motis/webservice/dataset_settings.h"
#include "motis/webservice/listener_settings.h"
#include "motis/webservice/modules_settings.h"

using namespace motis::webservice;
using namespace motis::module;
using namespace motis;

int main(int argc, char** argv) {
  message::init_parser();

  listener_settings listener_opt("0.0.0.0", "8080");
  dataset_settings dataset_opt("data/test");
  modules_settings modules_opt("modules");

  conf::options_parser parser({&listener_opt, &dataset_opt, &modules_opt});
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

  std::cout << "\n\tMOTIS Webservice\n\n";
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  auto sched = load_schedule(dataset_opt.dataset);

  boost::asio::io_service ios;

  ws_server server(ios);
  server.listen(listener_opt.host, listener_opt.port);

  dispatcher dispatcher(server);

#ifndef MOTIS_STATIC_MODULES
  dynamic_module_loader loader(modules_opt.path, sched.get(), dispatcher, ios);
  loader.load_modules();
#else
  namespace p = std::placeholders;
  send_fun send = std::bind(&dispatcher::send, &dispatcher, p::_1, p::_2);
  dispatch_fun dispatch =
      std::bind(&dispatcher::on_msg, &dispatcher, p::_1, p::_2);

  std::vector<std::unique_ptr<motis::module::module> > modules;
  modules.emplace_back(new guesser::guesser());
  modules.emplace_back(new railviz::railviz());
  modules.emplace_back(new routing::routing());
  modules.emplace_back(new reliability::reliability());

  for (auto const& module : modules) {
    dispatcher.modules_.push_back(module.get());
    for (auto const& subscription : module->subscriptions()) {
      dispatcher.subscriptions_.insert({subscription, module.get()});
    }

    module->schedule_ = sched.get();
    module->send_ = &send;
    module->dispatch_ = &dispatch;
    module->init();
  }
#endif

  std::vector<conf::configuration*> module_confs;
  for (auto const& module : dispatcher.modules_) {
    module_confs.push_back(module);
  }
  conf::options_parser module_conf_parser(module_confs);
  module_conf_parser.read_command_line_args(argc, argv);
  module_conf_parser.read_configuration_file();
  module_conf_parser.print_used(std::cout);

  using net::http::server::shutdown_handler;
  shutdown_handler<ws_server> server_shutdown_handler(ios, server);

  ios.run();
}
