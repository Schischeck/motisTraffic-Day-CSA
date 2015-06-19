#pragma once

#include <string>
#include <vector>

#include "json11/json11.hpp"

#include "conf/configuration.h"

#include "motis/module/sid.h"
#include "motis/core/schedule/Schedule.h"

namespace motis {
namespace module {

struct module : public conf::configuration {
  virtual std::string name() const = 0;
  virtual std::vector<json11::Json> on_msg(json11::Json const &, sid) {
    return {};
  }
  virtual void on_open(sid){};
  virtual void on_close(sid){};

  td::Schedule* schedule_;
};

} // namespace motis
} // namespace module