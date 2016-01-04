#pragma once

#include <string>
#include <memory>

#include "net/http/server/route_request_handler.hpp"

#include "motis/module/receiver.h"

namespace motis {
namespace launcher {

struct socket_server {
  socket_server(boost::asio::io_service&, motis::module::receiver&);
  ~socket_server();

  void listen(std::string const& host, std::string const& port);

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace launcher
}  // namespace motis
