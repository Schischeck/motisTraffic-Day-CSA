#pragma once

#include <map>
#include <memory>
#include <string>

#include "ctx/ctx.h"

#include "motis/module/access_t.h"

namespace motis {

struct schedule;

namespace module {

struct dispatcher;

struct ctx_data {
  ctx_data(access_t access, dispatcher* d, schedule* sched)
      : access_{access}, dispatcher_{d}, sched_{sched} {}

  void transition(ctx::transition, ctx::op_id const&, ctx::op_id const&) {}

  access_t access_;
  dispatcher* dispatcher_;
  schedule* sched_;
};

inline ctx_data& current_data() { return ctx::current_op<ctx_data>()->data_; }

}  // namespace module
}  // namespace motis
