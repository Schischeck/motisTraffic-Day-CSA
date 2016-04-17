#pragma once

#include "ctx/ctx.h"
#include "ctx/parallel_for.h"

namespace motis {
namespace module {

#define motis_parallel_for(vec, fn) \
  ctx::parallel_for<ctx_data>(vec, fn, ctx::op_id(CTX_LOCATION))

}  // namespace module
}  // namespace motis
