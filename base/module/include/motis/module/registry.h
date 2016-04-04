#pragma once

#include <map>

#include "motis/module/message.h"

namespace motis {
namespace module {

struct registry {
  using op = std::function<msg_ptr(msg_ptr const&)>;

  void register_op(std::string const& name, op fn) {
    if (!operations_.emplace(name, std::move(fn)).second) {
      throw std::runtime_error("target already registered");
    }
  }

  std::map<std::string, op> operations_;
};

}  // namespace module
}  // namespace motis
