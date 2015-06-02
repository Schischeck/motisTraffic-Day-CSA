#pragma once

#include <memory>

#include "boost/asio/io_service.hpp"

#include "json11/json11.hpp"

#include "motis/module/sid.h"

namespace motis {
namespace webservice {

struct ws_server_options {
  std::string host;
  std::string port;
};

struct ws_server final {
  ws_server(boost::asio::io_service& ios);
  ~ws_server();

  void on_msg(module::msg_handler);
  void on_open(module::sid_handler);
  void on_close(module::sid_handler);

  void listen(ws_server_options const& opt);
  void send(module::sid session, json11::Json const& msg);
  void stop();

  struct ws_server_impl;
  std::unique_ptr<ws_server_impl> impl_;
};

}  // namespace webservice
}  // namespace motis
