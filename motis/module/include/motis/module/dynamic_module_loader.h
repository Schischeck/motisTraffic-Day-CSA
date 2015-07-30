#pragma once

#include <string>

#include "boost/asio/io_service.hpp"
#include "boost/asio/signal_set.hpp"

#include "motis/core/schedule/schedule.h"

#include "motis/module/dispatcher.h"

namespace motis {
namespace module {

struct dynamic_module_loader {
  dynamic_module_loader(std::string const& modules_path,
                        motis::schedule* schedule, dispatcher& d,
                        boost::asio::io_service& ios,
                        boost::asio::io_service& thread_pool)
      : send_(std::bind(&dispatcher::send, &d, std::placeholders::_1,
                        std::placeholders::_2)),
        dispatch_(std::bind(&dispatcher::on_msg, &d, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3)),
        modules_path_(modules_path),
        schedule_(schedule),
        dispatcher_(d),
        context_({schedule, &ios, &thread_pool, &send_, &dispatch_}),
        signals_(ios) {
#if not defined(_WIN32) && not defined(_WIN64)
    signals_.add(SIGUSR1);
    listen_for_update_packages_signal();
#endif
  }

  void listen_for_update_packages_signal() {
    namespace p = std::placeholders;
    signals_.async_wait(
        std::bind(&dynamic_module_loader::reload, this, p::_1, p::_2));
  }

  void reload(boost::system::error_code /*ec*/, int /*signo*/) {
    load_modules();
    listen_for_update_packages_signal();
  }

  void load_modules() {
    dispatcher_.modules_.clear();
    dispatcher_.subscriptions_.clear();

    modules_ = modules_from_folder(modules_path_, &context_);
    for (auto const& module : modules_) {
      dispatcher_.modules_.push_back(module.module_.get());
      dispatcher_.add_module(module.module_.get());
      module.module_->init_(&context_);
    }

    print_modules();
  }

  void print_modules() {
    std::cout << "\nloaded modules: ";
    for (auto const& loaded_module : dispatcher_.modules_) {
      std::cout << loaded_module->name() << " ";
    }
    std::cout << std::endl;
  }

  send_fun send_;
  msg_handler dispatch_;
  std::vector<dynamic_module> modules_;
  std::string modules_path_;
  motis::schedule* schedule_;
  dispatcher& dispatcher_;
  context context_;
  boost::asio::signal_set signals_;
};

}  // namespace module
}  // namespace motis
