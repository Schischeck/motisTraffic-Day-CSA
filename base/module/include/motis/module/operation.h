#pragma once

#include <string>
#include <vector>
#include <functional>

#include "motis/module/future.h"

namespace motis {
namespace module {

struct operation {
  std::function<msg_ptr(msg_ptr const&)> fn_;
  std::vector<std::string> required_containers_;
  std::vector<std::string> required_operations_;
  std::string name_;
};

}  // namespace module
}  // namespace motis
