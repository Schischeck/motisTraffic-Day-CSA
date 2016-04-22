#pragma once

#include "ctx/ctx.h"
#include "ctx/parallel_for.h"

namespace motis {
namespace module {

#define motis_parallel_for(vec, fn) \
  ctx::parallel_for<ctx_data>(vec, fn, ctx::op_id(CTX_LOCATION))

template <typename T, typename Fn>
void spawn_job(T elem, Fn func) {
  auto& op = ctx::current_op<ctx_data>();
  auto id = ctx::op_id(CTX_LOCATION);
  id.parent_index = op.id_.index;
  op.sched_.post_void(op.data_, [&]() { func(elem); }, id);
}

}  // namespace module
}  // namespace motis
