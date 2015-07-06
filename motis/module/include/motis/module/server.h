#pragma once

#include "motis/module/handler_functions.h"

namespace motis {
namespace module {

struct server {
  virtual void on_msg(msg_handler) = 0;
  virtual void on_open(sid_handler) = 0;
  virtual void on_close(sid_handler) = 0;
  virtual void send(json11::Json const& msg, sid session) = 0;
};

}  // namespace module
}  // namespace motis