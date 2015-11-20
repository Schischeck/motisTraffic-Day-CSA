#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/thread.hpp"

#include "motis/core/schedule/time.h"

#include "motis/module/dispatcher.h"
#include "motis/module/module.h"
#include "motis/module/server.h"

#include "motis/reliability/context.h"
#include "motis/reliability/reliability.h"
#include "motis/routing/routing.h"

namespace motis {
namespace reliability {
struct start_and_travel_distributions;
namespace distriutions_container {
struct precomputed_distributions_container;
}
namespace system_tools {

struct setup {
  struct test_server : motis::module::server {
    void on_msg(module::msg_handler) override {}
    void on_open(module::sid_handler) override {}
    void on_close(module::sid_handler) override {}
    void send(module::msg_ptr const&, module::sid) override {}
  } t_;

  setup(schedule* sched, unsigned short const num_routing_modules = 1,
        bool const set_routing_num_max_label = true)
      : dispatcher_(t_, ios_) {
    namespace p = std::placeholders;
    dispatch_ = std::bind(&module::dispatcher::on_msg, &dispatcher_, p::_1,
                          p::_2, p::_3);
    c_.schedule_ = sched;
    c_.thread_pool_ = num_routing_modules > 1 ? &thread_pool_ : &ios_;
    c_.ios_ = &ios_;
    c_.dispatch_ = &dispatch_;

    std::cout << "\ncreate " << num_routing_modules << " routing modules"
              << std::endl;
    for (unsigned int i = 0; i < num_routing_modules; ++i) {
      modules_.emplace_back(set_routing_num_max_label
                                ? new routing::routing(100)
                                : new routing::routing());
    }
    modules_.emplace_back(new reliability());
    for (auto const& module : modules_) {
      dispatcher_.modules_.push_back(module.get());
      dispatcher_.add_module(module.get());
      module->init_(&c_);
    }

    if (num_routing_modules > 1) {
      tp_work_ = std::unique_ptr<boost::asio::io_service::work>(
          new boost::asio::io_service::work(thread_pool_));
      ios_work_ = std::unique_ptr<boost::asio::io_service::work>(
          new boost::asio::io_service::work(ios_));
      threads_.resize(8);
      auto run = [&]() {
        try {
          thread_pool_.run();
        } catch (std::exception const& e) {
          std::cout << "unhandled error: " << e.what();
          exit(0);
        }
      };

      for (unsigned i = 0; i < threads_.size(); ++i) {
        threads_[i] = boost::thread(run);
      }
    }

    reliability_context_ = std::unique_ptr<motis::reliability::context>(
        new motis::reliability::context(
            *sched, reliability_module().precomputed_distributions(),
            reliability_module().s_t_distributions()));
  }

  boost::asio::io_service ios_, thread_pool_;
  std::unique_ptr<boost::asio::io_service::work> tp_work_, ios_work_;
  std::vector<boost::thread> threads_;
  module::dispatcher dispatcher_;
  module::msg_handler dispatch_;
  motis::module::context c_;
  std::vector<std::unique_ptr<motis::module::module> > modules_;
  std::unique_ptr<motis::reliability::context> reliability_context_;

  reliability& reliability_module() {
    return dynamic_cast<reliability&>(*modules_.back());
  }
};  // struct setup
}  // namespace system
}  // namespace reliability
}  // namespace motis
