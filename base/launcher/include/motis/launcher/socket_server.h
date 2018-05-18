#pragma once

#include <memory>
#include <string>

#include "net/http/server/route_request_handler.hpp"

#include "motis/module/receiver.h"

namespace motis {
namespace launcher {

struct socket_server {
  socket_server(boost::asio::io_service&, motis::module::receiver&);
  ~socket_server();

  socket_server(socket_server&&) = default;
  socket_server& operator=(socket_server&&) = default;

  socket_server(socket_server const&) = delete;
  socket_server& operator=(socket_server const&) = delete;

  void listen(std::string const& host, std::string const& port);
  void stop();

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace launcher
}  // namespace motis
