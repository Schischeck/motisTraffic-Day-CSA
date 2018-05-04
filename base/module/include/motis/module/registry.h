#pragma once

#include <map>

#include "motis/core/common/logging.h"

#include "motis/module/access_t.h"
#include "motis/module/message.h"

namespace motis {

struct schedule;

namespace module {

struct op {
  op(std::function<msg_ptr(msg_ptr const&)> fn, access_t access)
      : fn_{std::move(fn)}, access_{access} {}
  std::function<msg_ptr(msg_ptr const&)> fn_;
  access_t access_;
};

struct registry {
  template <typename Fn>
  void register_op(std::string const& name, Fn fn,
                   access_t const access = access_t::READ) {
    auto const call =
        [ fn_rec = std::forward<Fn>(fn), name ](msg_ptr const& m)->msg_ptr {
      logging::scoped_timer t{name};
      return fn_rec(m);
    };
    if (!operations_.emplace(name, op{std::move(call), access}).second) {
      throw std::runtime_error("target already registered");
    }
  }

  template <typename Fn>
  void subscribe(std::string const& topic, Fn fn,
                 access_t const access = access_t::READ) {
    topic_subscriptions_[topic].emplace_back(
        [ fn_rec = std::forward<Fn>(fn), topic ](msg_ptr const& m)->msg_ptr {
          logging::scoped_timer t{topic};
          return fn_rec(m);
        },
        access);
  }

  template <typename Fn>
  void subscribe_void(std::string const& topic, Fn fn,
                      access_t const access = access_t::READ) {
    subscribe(
        topic,
        [ fn_rec = std::forward<Fn>(fn), topic ](msg_ptr const&)->msg_ptr {
          logging::scoped_timer t{topic};
          fn_rec();
          return nullptr;
        },
        access);
  }

  schedule* sched_ = nullptr;
  std::map<std::string, op> operations_;
  std::map<std::string, std::vector<op>> topic_subscriptions_;
};

}  // namespace module
}  // namespace motis
