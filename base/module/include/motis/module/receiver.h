#pragma once

#include "motis/module/callbacks.h"

namespace motis {
namespace module {

class receiver {
public:
  virtual ~receiver() {}
  virtual void on_msg(msg_ptr msg, sid session, callback cb, bool locked) = 0;
  virtual void on_open(sid session) = 0;
  virtual void on_close(sid session) = 0;
};

}  // namespace module
}  // namespace motis
