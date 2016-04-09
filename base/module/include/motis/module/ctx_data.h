#pragma once

#include <map>
#include <memory>
#include <string>

#include "ctx/ctx.h"

namespace motis {

struct schedule;

namespace module {

struct dispatcher;

using env_table = std::shared_ptr<std::map<std::string, std::string>>;

struct ctx_data {
  ctx_data(dispatcher* d, schedule* sched, env_table e = env_table())
      : dispatcher_(d), sched_(sched), env_(std::move(e)) {}

  dispatcher* dispatcher_;
  schedule* sched_;
  env_table env_;

  void transition(ctx::transition, ctx::op_id, ctx::op_id) {}
};

inline ctx_data& current_data() { return ctx::current_op<ctx_data>().data_; }

}  // namespace module
}  // namespace motis
