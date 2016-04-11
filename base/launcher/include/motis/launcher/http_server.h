#pragma once

#include <memory>
#include <string>

#include "net/http/server/route_request_handler.hpp"

#include "motis/module/receiver.h"

namespace motis {
namespace launcher {

struct http_server {
  http_server(boost::asio::io_service&, motis::module::receiver&);
  ~http_server();

  void listen(std::string const& host, std::string const& port);
  void stop();

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace launcher
}  // namespace motis
