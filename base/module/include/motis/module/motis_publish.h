#pragma once

#include "ctx/ctx.h"

#include "motis/module/ctx_data.h"
#include "motis/module/message.h"
#include "motis/module/dispatcher.h"
#include "motis/module/future.h"

namespace motis {
namespace module {

inline std::vector<future> motis_publish_(msg_ptr const& msg, ctx::op_id id) {
  auto& op = ctx::current_op<ctx_data>();
  auto& data = op.data_;
  id.parent_index = op.id_.index;
  return data.dispatcher_->publish(msg, data, id);
}

#define motis_publish(msg) motis_publish_(msg, ctx::op_id(CTX_LOCATION))

}  // namespace module
}  // namespace motis
