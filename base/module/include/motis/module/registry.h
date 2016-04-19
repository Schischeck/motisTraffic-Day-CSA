#pragma once

#include <map>

#include "motis/module/message.h"

namespace motis {

struct schedule;

namespace module {

using op = std::function<msg_ptr(msg_ptr const&)>;

struct registry {
  template <typename Fn>
  void register_op(std::string const& name, Fn fn) {
    if (!operations_.emplace(name, std::forward<Fn>(fn)).second) {
      throw std::runtime_error("target already registered");
    }
  }

  template <typename Fn>
  void subscribe(std::string const& topic, Fn fn) {
    topic_subscriptions_[topic].emplace_back(std::forward<Fn>(fn));
  }

  schedule* sched_;
  std::map<std::string, op> operations_;
  std::map<std::string, std::vector<op>> topic_subscriptions_;
};

}  // namespace module
}  // namespace motis
