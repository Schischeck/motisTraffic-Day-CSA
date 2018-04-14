#pragma once

#include "ctx/ctx.h"

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

struct dispatcher : public receiver {
  dispatcher(boost::asio::io_service& ios, registry& reg)
      : scheduler_(ios), registry_(reg) {}

  std::vector<future> publish(msg_ptr const& msg, ctx_data const& data,
                              ctx::op_id id) {
    id.name = msg->get()->destination()->target()->str();
    auto it = registry_.topic_subscriptions_.find(id.name);
    if (it == end(registry_.topic_subscriptions_)) {
      return {};
    }
    return utl::to_vec(it->second, [&](auto&& subscriber) {
      return scheduler_.post(data, std::bind(subscriber.fn_, msg), id);
    });
  }

  future req(msg_ptr const& msg, ctx_data const& data, ctx::op_id id) {
    try {
      id.name = msg->get()->destination()->target()->str();
      return scheduler_.post(
          data, std::bind(registry_.operations_.at(id.name).fn_, msg), id);
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
    return scheduler_.enqueue(
        ctx_data(this, registry_.sched_),
        [this, id, cb, msg]() {
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
        },
        id);
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

  ctx::scheduler<ctx_data> scheduler_;
  registry& registry_;
};

}  // namespace module
}  // namespace motis
