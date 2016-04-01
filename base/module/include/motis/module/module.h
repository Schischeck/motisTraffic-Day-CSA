#pragma once

#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"

#include "conf/configuration.h"

#include "motis/core/schedule/synced_schedule.h"

#include "motis/module/message.h"
#include "motis/module/registry.h"

namespace motis {
namespace module {

struct module : public conf::configuration {
  virtual ~module() {}

  virtual std::string name() const = 0;
  virtual void init(registry& reg) {}

protected:
  template <schedule_access A>
  synced_schedule<A> synced_sched() {
    return synced_schedule<A>(*schedule_);
  }

private:
  motis::schedule* schedule_;
};

}  // namespace module
}  // namespace motis
