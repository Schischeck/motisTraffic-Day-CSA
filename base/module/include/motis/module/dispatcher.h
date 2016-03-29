#pragma once

#include "ctx/scheduler.h"

#include "motis/module/ctx_data.h"
#include "motis/module/future.h"

namespace motis {
namespace module {

struct dispatcher {
  future req(msg_ptr const& req) { return scheduler_.post(); }

  future req_with_fresh_snapshot(msg_ptr const& req) {
    return scheduler_.post();
  }

  ctx::scheduler<ctx_data> scheduler_;
};

}  // namespace module
}  // namespace motis
