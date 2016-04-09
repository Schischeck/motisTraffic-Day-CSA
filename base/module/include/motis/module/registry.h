#pragma once

#include <map>

#include "motis/module/message.h"

namespace motis {

struct schedule;

namespace module {

using op = std::function<msg_ptr(msg_ptr const&)>;

namespace detail {
// void fn(void)
template <typename Fn>
inline auto wrap_op(Fn fn) ->
    typename std::enable_if<std::is_same<decltype(fn()), void>::value,
                            op>::type {
  return [fn = std::move(fn)](msg_ptr const&) {
    fn();
    return make_success_msg();
  };
}

// msg_ptr fn(void)
template <typename Fn>
inline auto wrap_op(Fn fn) ->
    typename std::enable_if<std::is_same<decltype(fn()), msg_ptr>::value,
                            op>::type {
  return [fn = std::move(fn)](msg_ptr const&) { return fn(); };
}

// void fn(msg_ptr)
template <typename Fn>
inline auto wrap_op(Fn fn) -> typename std::enable_if<
    std::is_same<decltype(fn(make_success_msg())), void>::value, op>::type {
  return [fn = std::move(fn)](msg_ptr const& msg) {
    fn(msg);
    return make_success_msg();
  };
}

// msg_ptr fn(msg_ptr)
template <typename Fn>
inline auto wrap_op(Fn fn) -> typename std::enable_if<
    std::is_same<decltype(fn(make_success_msg())), msg_ptr>::value, op>::type {
  return [fn = std::move(fn)](msg_ptr const& msg) { return fn(msg); };
}

}  // namespace detail

struct registry {
  template <typename Fn>
  void register_op(std::string const& name, Fn fn) {
    if (!operations_.emplace(name, detail::wrap_op(std::move(fn))).second) {
      throw std::runtime_error("target already registered");
    }
  }

  template <typename Fn>
  void subscribe(std::string const& topic, Fn fn) {
    topic_subscriptions_[topic].emplace_back(detail::wrap_op(std::move(fn)));
  }

  schedule* sched_;
  std::map<std::string, op> operations_;
  std::map<std::string, std::vector<op>> topic_subscriptions_;
};

}  // namespace module
}  // namespace motis
