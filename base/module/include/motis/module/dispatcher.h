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

struct dispatcher : public receiver, public ctx::access_scheduler<ctx_data> {
  dispatcher(boost::asio::io_service& ios, registry& reg)
      : ctx::access_scheduler<ctx_data>(ios), registry_(reg) {}

  std::vector<future> publish(msg_ptr const& msg, ctx_data const& data,
                              ctx::op_id id) {
    id.name = msg->get()->destination()->target()->str();
    auto it = registry_.topic_subscriptions_.find(id.name);
    if (it == end(registry_.topic_subscriptions_)) {
      return {};
    }
    return utl::to_vec(it->second, [&](auto&& op) {
      verify(ctx::current_op<ctx_data>() == nullptr ||
                 ctx::current_op<ctx_data>()->data_.access_ == op.access_,
             "matches the access permissions of parent or be root operation");
      return post(data, std::bind(op.fn_, msg), id);
    });
  }

  future req(msg_ptr const& msg, ctx_data const& data, ctx::op_id id) {
    try {
      id.name = msg->get()->destination()->target()->str();
      auto const& op = registry_.operations_.at(id.name);
      verify(ctx::current_op<ctx_data>() == nullptr ||
                 ctx::current_op<ctx_data>()->data_.access_ == op.access_,
             "matches the access permissions of parent or be root operation");
      id.name = msg->get()->destination()->target()->str();
      return post(data, std::bind(op.fn_, msg), id);
    } catch (std::out_of_range const&) {
      throw std::system_error(error::target_not_found);
    }
  }

  void on_msg(msg_ptr const& msg, callback const& cb) override {
    if (!std::strcmp("/api", msg->get()->destination()->target()->c_str())) {
      return cb(api(), std::error_code());
    }

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

  msg_ptr api() const {
    message_creator fbb;

    std::vector<char const*> fbs_def;
    auto def = message::get_fbs_definitions();
    fbs_def.reserve(def.second);
    for (auto i = 0u; i < def.second; ++i) {
      fbs_def.emplace_back(def.first[i]);
    }

    struct method {
      method(std::string path, std::string in, std::string out)
          : path_(std::move(path)), in_(std::move(in)), out_(std::move(out)) {}
      std::string path_, in_, out_;
    };
    std::vector<method> methods = {
        {"/routing", "RoutingRequest", "RoutingResponse"},
        {"/guesser", "StationGuesserRequest", "StationGuesserResponse"}};

    fbb.create_and_finish(
        MsgContent_ApiDescription,
        CreateApiDescription(
            fbb,
            fbb.CreateVector(utl::to_vec(
                fbs_def,
                [&fbb](char const* str) { return fbb.CreateString(str); })),
            fbb.CreateVector(utl::to_vec(methods,
                                         [&fbb](method const& m) {
                                           return CreateMethod(
                                               fbb, fbb.CreateString(m.path_),
                                               fbb.CreateString(m.in_),
                                               fbb.CreateString(m.out_));
                                         })))
            .Union());
    return make_msg(fbb);
  }

  registry& registry_;
};

}  // namespace module
}  // namespace motis
