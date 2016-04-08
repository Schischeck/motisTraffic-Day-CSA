#pragma once

#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"

#include "conf/simple_config.h"

#include "motis/core/schedule/synced_schedule.h"

#include "motis/module/message.h"
#include "motis/module/registry.h"

namespace motis {
namespace module {

struct module : public conf::simple_config {
  module(std::string name, std::string prefix)
      : simple_config(std::move(name), std::move(prefix)),
        schedule_(nullptr),
        ios_(nullptr) {}

  module() : module("", "") {}

  virtual ~module() {}

  void set_context(motis::schedule& schedule, boost::asio::io_service& ios) {
    schedule_ = &schedule;
    ios_ = &ios;
  }

  virtual std::string name() const = 0;
  virtual void init(registry&) {}

protected:
  template <schedule_access A>
  synced_schedule<A> synced_sched() {
    return synced_schedule<A>(*schedule_);
  }

private:
  motis::schedule* schedule_;
  boost::asio::io_service* ios_;
};

}  // namespace module
}  // namespace motis
