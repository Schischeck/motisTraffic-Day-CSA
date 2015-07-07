#pragma once

#include <functional>
#include <map>

#include "motis/module/module.h"

#include "motis/protocol/Message_generated.h"

#include "motis/module/server.h"

namespace motis {
namespace module {

struct dispatcher {
  dispatcher(server& server) : server_(server) {
    namespace p = std::placeholders;
    server_.on_msg(std::bind(&dispatcher::on_msg, this, p::_1, p::_2));
    server_.on_open(std::bind(&dispatcher::on_open, this, p::_1));
    server_.on_close(std::bind(&dispatcher::on_close, this, p::_1));
  }

  void send(msg_ptr const& msg, sid session) {
    server_.send(msg, session);
  }

  msg_ptr on_msg(msg_ptr const& msg, sid session) {
    auto module_it = subscriptions_.find(msg->msg_->content_type());
    if (module_it == end(subscriptions_)) {
      return {};
    }
    auto response = module_it->second->on_msg(msg, session);
    return response;
  }

  void on_open(sid session) {
    for (auto const& module : modules_) {
      module->on_open(session);
    }
  }

  void on_close(sid session) {
    for (auto const& module : modules_) {
      module->on_close(session);
    }
  }

  server& server_;
  std::vector<module*> modules_;
  std::map<MsgContent, module*> subscriptions_;
};

}  // namespace module
}  // namespace motis
