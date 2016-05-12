#pragma once

#include "ctx/ctx.h"

#include "motis/module/ctx_data.h"

namespace motis {
namespace module {

template <typename Fn>
auto spawn_job(Fn&& fn) {
  auto& op = ctx::current_op<ctx_data>();
  auto id = ctx::op_id(CTX_LOCATION);
  id.parent_index = op.id_.index;
  return op.sched_.post(op.data_, std::forward<Fn>(fn), id);
}

}  // namespace module
}  // namespace motis
