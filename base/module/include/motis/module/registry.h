#pragma once

#include <map>

#include "motis/module/container.h"
#include "motis/module/operation.h"

namespace motis {
namespace module {

struct registry {
  void register_op(std::string name, std::function<msg_ptr(msg_ptr)> fn) {
    if (!operations_.emplace(name, operation(name, std::move(fn))).second) {
      throw std::runtime_error("target already registered");
    }
  }

  std::map<std::string, operation> operations_;
  snapshot containers_;
};

}  // namespace module
}  // namespace motis
