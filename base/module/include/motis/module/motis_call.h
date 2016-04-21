#pragma once

#include "ctx/ctx.h"

#include "motis/module/ctx_data.h"
#include "motis/module/message.h"
#include "motis/module/dispatcher.h"
#include "motis/module/future.h"

namespace motis {
namespace module {

inline future motis_call_(msg_ptr const& msg, ctx::op_id id) {
  auto& op = ctx::current_op<ctx_data>();
  auto& data = op.data_;
  id.parent_index = op.id_.index;
  return data.dispatcher_->req(msg, data, id);
}

#define motis_call(msg) motis_call_(msg, ctx::op_id(CTX_LOCATION))

}  // namespace module
}  // namespace motis
