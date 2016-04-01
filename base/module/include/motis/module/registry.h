#pragma once

#include <map>

#include "motis/module/container.h"
#include "motis/module/message.h"

namespace motis {
namespace module {

struct registry {
  void register_op(std::string name, std::function<msg_ptr(msg_ptr)> fn) {
    if (!operations_.emplace(name, std::move(fn)).second) {
      throw std::runtime_error("target already registered");
    }
  }

  std::map<std::string, std::function<msg_ptr(msg_ptr const&)>> operations_;
  snapshot containers_;
};

}  // namespace module
}  // namespace motis
