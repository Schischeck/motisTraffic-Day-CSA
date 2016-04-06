#pragma once

namespace motis {
namespace module {

inline schedule& get_schedule() {
  return *ctx::current_op<ctx_data>().data_.dispatcher_->registry_.sched_;
}

}  // namespace module
}  // namespace motis
