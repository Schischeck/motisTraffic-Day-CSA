#pragma once

#include <system_error>

#include "motis/module/message.h"

namespace motis {
namespace module {

using callback = std::function<void(msg_ptr, std::error_code)>;

struct receiver {
  virtual ~receiver() = default;
  virtual void on_msg(msg_ptr const&, callback const&) = 0;
};

}  // namespace module
}  // namespace motis
