#pragma once

#include "ctx/ctx.h"

#include "utl/parser/util.h"
#include "utl/to_vec.h"

#include "motis/core/common/logging.h"

#include "motis/module/ctx_data.h"
#include "motis/module/error.h"
#include "motis/module/future.h"
#include "motis/module/message.h"
#include "motis/module/receiver.h"
#include "motis/module/registry.h"

namespace motis {
namespace module {

/* Concurrency model:
 *   - multiple reads or one single write at a time
 *   - parent op needs to have equal or more permissions as child
 *   - parent op may not quit until all child ops that require
 *     access permissions have finished
 *   - (internal) -> child ops do not require own access permissions
 *   - note: write ops could spawn multiple parallel write child
 *           ops which leads to (unchecked) data-races!
 */
struct dispatcher : public receiver, public ctx::access_scheduler<ctx_data> {
  dispatcher(boost::asio::io_service& ios, registry& reg)
      : ctx::access_scheduler<ctx_data>(ios), registry_(reg) {}

  access_t access_of(std::string const& target) {
    if (auto const it = registry_.topic_subscriptions_.find(target);
        it != end(registry_.topic_subscriptions_)) {
      verify(!registry_.topic_subscriptions_.empty(),
             "empty topic subscriptions");
      return std::max_element(
                 begin(it->second), end(it->second),
                 [](auto const& a, auto const& b) {
                   return static_cast<std::underlying_type_t<access_t>>(
                              a.access_) <
                          static_cast<std::underlying_type_t<access_t>>(
                              b.access_);
                 })
          ->access_;
    } else if (auto const it = registry_.operations_.find(target);
               it != end(registry_.operations_)) {
      return it->second.access_;
    } else {
      return access_t::READ;
    }
  }

  access_t access_of(msg_ptr const& msg) {
    return access_of(msg->get()->destination()->target()->str());
  }

  std::vector<future> publish(msg_ptr const& msg, ctx_data const& data,
                              ctx::op_id id) {
    id.name = msg->get()->destination()->target()->str();
    auto it = registry_.topic_subscriptions_.find(id.name);
    if (it == end(registry_.topic_subscriptions_)) {
      return {};
    }

    return utl::to_vec(it->second, [&](auto&& op) {
      verify(ctx::current_op<ctx_data>() == nullptr ||
                 ctx::current_op<ctx_data>()->data_.access_ >= op.access_,
             "matches the access permissions of parent or be root operation");
      return post(data, std::bind(op.fn_, msg), id);
    });
  }

  future req(msg_ptr const& msg, ctx_data const& data, ctx::op_id id) {
    try {
      id.name = msg->get()->destination()->target()->str();
      auto const& op = registry_.operations_.at(id.name);
      verify(ctx::current_op<ctx_data>() == nullptr ||
                 ctx::current_op<ctx_data>()->data_.access_ >= op.access_,
             "matches the access permissions of parent or be root operation");
      id.name = msg->get()->destination()->target()->str();
      return post(data, std::bind(op.fn_, msg), id);
    } catch (std::out_of_range const&) {
      throw std::system_error(error::target_not_found);
    }
  }

  void on_msg(msg_ptr const& msg, callback const& cb) override {
    ctx::op_id id;
    id.created_at = "dispatcher::on_msg";
    id.parent_index = 0;
    id.name = msg->get()->destination()->target()->str();

    auto fn = [this, id, cb, msg]() {
      try {
        return cb(registry_.operations_.at(id.name).fn_(msg),
                  std::error_code());
      } catch (std::system_error const& e) {
        return cb(nullptr, e.code());
      } catch (std::out_of_range const&) {
        LOG(logging::log_level::error)
            << "target \"" << id.name << "\" not found";
        return cb(nullptr, error::target_not_found);
      } catch (...) {
        return cb(nullptr, error::unknown_error);
      }
    };

    auto const op_it = registry_.operations_.find(id.name);
    if (op_it == end(registry_.operations_)) {
      return cb(nullptr, error::target_not_found);
    }

    auto const data = ctx_data(op_it->second.access_, this, registry_.sched_);
    return op_it->second.access_ == access_t::READ
               ? enqueue_read(data, fn, id)
               : enqueue_write(data, fn, id);
  }

  registry& registry_;
};

}  // namespace module
}  // namespace motis
