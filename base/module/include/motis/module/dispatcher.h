#pragma once

#include "ctx/ctx.h"

#include "motis/module/registry.h"
#include "motis/module/receiver.h"
#include "motis/module/ctx_data.h"
#include "motis/module/future.h"
#include "motis/module/error.h"

namespace motis {
namespace module {

struct dispatcher : public receiver {
  dispatcher(boost::asio::io_service& ios, registry& reg)
      : scheduler_(ios), registry_(reg) {}

  void on_msg(msg_ptr msg, callback cb) override {
    ctx::op_id id;
    id.created_at = "dispatcher::on_msg";
    id.parent_index = 0;
    id.name = msg->get()->destination()->target()->str();
    return scheduler_.enqueue(ctx_data(this), [this, id, cb, msg]() {
      try {
        auto fn = registry_.operations_.at(id.name);
        auto res = fn(msg);
        return cb(res, boost::system::error_code());
      } catch (boost::system::system_error const& e) {
        return cb(nullptr, e.code());
      } catch (std::out_of_range const&) {
        return cb(nullptr, error::target_not_found);
      } catch (...) {
        return cb(nullptr, error::unknown_error);
      }
    }, id);
  }

  future req(msg_ptr const& msg, ctx_data const& data, ctx::op_id id) {
    try {
      id.name = msg->get()->destination()->target()->str();
      return scheduler_.post(
          data, std::bind(registry_.operations_.at(id.name), msg), id);
    } catch (std::out_of_range const&) {
      throw boost::system::system_error(error::target_not_found);
    }
  }

  ctx::scheduler<ctx_data> scheduler_;
  registry& registry_;
};

}  // namespace module
}  // namespace motis
