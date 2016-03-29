#pragma once

#include <string>
#include <vector>
#include <functional>

#include "motis/module/future.h"

namespace motis {
namespace module {

struct operation {
  std::string name_;
  std::function<future(msg_ptr const&)> fn_;
  std::vector<std::string> required_containers_;
  std::vector<std::string> required_operations_;
};

}  // namespace module
}  // namespace motis
