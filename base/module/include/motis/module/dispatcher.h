#pragma once

#include "ctx/ctx.h"

#include "motis/module/ctx_data.h"
#include "motis/module/error.h"
#include "motis/module/future.h"
#include "motis/module/receiver.h"
#include "motis/module/registry.h"

namespace motis {
namespace module {

struct dispatcher : public receiver {
  dispatcher(boost::asio::io_service& ios, registry& reg)
      : scheduler_(ios), registry_(reg) {}

  std::vector<future> publish(msg_ptr const& msg, ctx_data const& data,
                              ctx::op_id id) {
    id.name = msg->get()->destination()->target()->str();
    std::vector<future> futures;
    auto it = registry_.topic_subscriptions_.find(id.name);
    if (it != end(registry_.topic_subscriptions_)) {
      for (auto& subscriber : it->second) {
        futures.emplace_back(
            scheduler_.post(data, std::bind(subscriber, msg), id));
      }
    }
    return futures;
  }

  future req(msg_ptr const& msg, ctx_data const& data, ctx::op_id id) {
    try {
      id.name = msg->get()->destination()->target()->str();
      return scheduler_.post(
          data, std::bind(registry_.operations_.at(id.name), msg), id);
    } catch (std::out_of_range const&) {
      throw std::system_error(error::target_not_found);
    }
  }

  void on_msg(msg_ptr const& msg, callback const& cb) override {
    ctx::op_id id;
    id.created_at = "dispatcher::on_msg";
    id.parent_index = 0;
    id.name = msg->get()->destination()->target()->str();
    return scheduler_.enqueue(ctx_data(this, registry_.sched_),
                              [this, id, cb, msg]() {
                                try {
                                  return cb(
                                      registry_.operations_.at(id.name)(msg),
                                      std::error_code());
                                } catch (std::system_error const& e) {
                                  return cb(nullptr, e.code());
                                } catch (std::out_of_range const&) {
                                  return cb(nullptr, error::target_not_found);
                                } catch (...) {
                                  return cb(nullptr, error::unknown_error);
                                }
                              },
                              id);
  }

  ctx::scheduler<ctx_data> scheduler_;
  registry& registry_;
};

}  // namespace module
}  // namespace motis
