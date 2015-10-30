#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/core/schedule/time.h"

#include "motis/module/dispatcher.h"
#include "motis/module/module.h"
#include "motis/module/server.h"

#include "motis/reliability/reliability.h"
#include "motis/routing/routing.h"

namespace motis {
namespace reliability {
namespace system_tools {

struct setup {
  struct test_server : motis::module::server {
    void on_msg(module::msg_handler) override {}
    void on_open(module::sid_handler) override {}
    void on_close(module::sid_handler) override {}
    void send(module::msg_ptr const&, module::sid) override {}
  } t;

  boost::asio::io_service ios;
  module::dispatcher dispatcher;
  module::msg_handler dispatch;
  motis::module::context c;
  std::vector<std::unique_ptr<motis::module::module> > modules;

  setup(schedule* sched) : dispatcher(t, ios) {
    namespace p = std::placeholders;
    dispatch = std::bind(&module::dispatcher::on_msg, &dispatcher, p::_1, p::_2,
                         p::_3);
    c.schedule_ = sched;
    c.thread_pool_ = &ios;
    c.ios_ = &ios;
    c.dispatch_ = &dispatch;

    modules.emplace_back(new routing::routing());
    modules.emplace_back(new reliability());
    for (auto const& module : modules) {
      dispatcher.modules_.push_back(module.get());
      dispatcher.add_module(module.get());
      module->init_(&c);
    }
  }

  reliability& reliability_module() {
    return dynamic_cast<reliability&>(*modules[1]);
  }
};  // struct setup
}  // namespace system
}  // namespace reliability
}  // namespace motis
