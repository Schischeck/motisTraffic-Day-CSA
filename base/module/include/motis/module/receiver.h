#pragma once

#include "boost/system/error_code.hpp"

#include "motis/module/message.h"

namespace motis {
namespace module {

using callback = std::function<void(msg_ptr, boost::system::error_code)>;

struct receiver {
  virtual ~receiver() {}
  virtual void on_msg(msg_ptr, callback) = 0;
};

}  // namespace module
}  // namespace motis
