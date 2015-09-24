#pragma once

#include <memory>

#include "boost/asio/io_service.hpp"

#include "motis/module/server.h"

namespace motis {
namespace webservice {

struct ws_server : public module::server {
  ws_server(boost::asio::io_service& ios);
  ~ws_server();

  virtual void on_msg(module::msg_handler) override;
  virtual void on_open(module::sid_handler) override;
  virtual void on_close(module::sid_handler) override;
  virtual void send(motis::module::msg_ptr const& msg,
                    module::sid session) override;

  void listen(std::string const& host, std::string const& port);
  void stop();

  struct ws_server_impl;
  std::unique_ptr<ws_server_impl> impl_;
};

}  // namespace webservice
}  // namespace motis
