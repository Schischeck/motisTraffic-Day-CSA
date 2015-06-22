#pragma once

#include <functional>
#include <map>

#include "json11/json11.hpp"

#include "motis/module/module.h"

#include "motis/module/sid.h"

namespace motis {
namespace module {

template <typename Server>
struct dispatcher {
  dispatcher(Server& server) : server_(server) {
    namespace p = std::placeholders;
    server.on_msg(std::bind(&dispatcher::on_msg, this, p::_1, p::_2));
    server.on_open(std::bind(&dispatcher::on_open, this, p::_1));
    server.on_close(std::bind(&dispatcher::on_close, this, p::_1));
  }

  void send(sid session, json11::Json const& msg) {
    server_.send(session, msg);
  }

  json11::Json on_msg(json11::Json const& msg, sid session) {
    auto module_name = msg["module"].string_value();
    auto module_it = modules_.find(module_name);
    if (module_it == end(modules_)) {
      return {};
    }
    auto res = module_it->second->on_msg(msg, session);
    return res;
  }

  void on_open(sid session) {
    for (auto const& module : modules_) {
      module.second->on_open(session);
    }
  }

  void on_close(sid session) {
    for (auto const& module : modules_) {
      module.second->on_close(session);
    }
  }

  Server& server_;
  std::map<std::string, module*> modules_;
};

}  // namespace module
}  // namespace motis
