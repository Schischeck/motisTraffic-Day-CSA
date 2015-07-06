#pragma once

#include <string>
#include <vector>

#include "json11/json11.hpp"

#include "conf/configuration.h"

#include "motis/core/schedule/schedule.h"

#include "motis/module/sid.h"
#include "motis/module/handler_functions.h"

namespace motis {
namespace module {

struct module : public conf::configuration {
  virtual std::string name() const = 0;

  virtual void init() {}
  virtual json11::Json on_msg(json11::Json const&, sid) { return {}; }
  virtual void on_open(sid){};
  virtual void on_close(sid){};

  send_fun* send_;
  motis::schedule* schedule_;
};

}  // namespace motis
}  // namespace module
