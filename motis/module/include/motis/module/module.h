#pragma once

#include <string>
#include <vector>

#include "conf/configuration.h"

#include "motis/core/schedule/schedule.h"

#include "motis/module/sid.h"
#include "motis/module/message.h"
#include "motis/module/handler_functions.h"

namespace motis {
namespace module {

struct module : public conf::configuration {
  virtual ~module() {}
  virtual std::string name() const = 0;
  virtual std::vector<MsgContent> subscriptions() const = 0;
  virtual void init() {}
  virtual msg_ptr on_msg(msg_ptr const&, sid) { return {}; }
  virtual void on_open(sid){};
  virtual void on_close(sid){};

  send_fun* send_;
  dispatch_fun* dispatch_;
  motis::schedule* schedule_;
};

}  // namespace motis
}  // namespace module
