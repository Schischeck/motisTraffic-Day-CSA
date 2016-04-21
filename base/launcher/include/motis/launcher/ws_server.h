#pragma once

#include <memory>

#include "boost/asio/io_service.hpp"

#include "motis/module/receiver.h"

namespace motis {
namespace launcher {

struct ws_server {
  ws_server(boost::asio::io_service& ios, motis::module::receiver&);
  ~ws_server();

  void set_api_key(std::string const&);

  void send(motis::module::msg_ptr const& msg, unsigned session);

  void listen(std::string const& host, std::string const& port);
  void stop();

  struct ws_server_impl;
  std::unique_ptr<ws_server_impl> impl_;
};

}  // namespace launcher
}  // namespace motis
