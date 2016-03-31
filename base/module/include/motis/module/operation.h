#pragma once

#include <string>
#include <vector>
#include <functional>

#include "motis/module/future.h"

namespace motis {
namespace module {

struct operation {
  operation(std::string name, std::function<msg_ptr(msg_ptr const&)> fn)
      : name_(std::move(name)), fn_(std::move(fn)) {}

  std::string name_;
  std::function<msg_ptr(msg_ptr const&)> fn_;
};

}  // namespace module
}  // namespace motis
