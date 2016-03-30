#pragma once

#include "ctx/operation.h"

#include "motis/module/container.h"

namespace motis {
namespace module {

struct dispatcher;

using env_table = std::shared_ptr<std::map<std::string, std::string>>;

struct ctx_data {
  ctx_data(dispatcher* d, std::shared_ptr<snapshot> s, env_table e)
      : dispatcher_(d), snapshot_(std::move(s)), env_(std::move(e)) {}

  dispatcher* dispatcher_;
  std::shared_ptr<snapshot> snapshot_;
  env_table env_;

  void transition(ctx::transition, ctx::op_id, ctx::op_id) {}
};

inline ctx_data& current_data() { return ctx::current_op<ctx_data>().data_; }

}  // namespace module
}  // namespace motis
