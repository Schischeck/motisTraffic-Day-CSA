#include "motis/webservice/ws_server.h"

#include <memory>
#include <functional>

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> asio_ws_server;

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::error_code;
using websocketpp::frame::opcode::TEXT;

using namespace motis::module;

namespace motis {
namespace webservice {

struct ws_server::ws_server_impl {
  ws_server_impl(boost::asio::io_service& ios)
      : ios_(ios),
        next_sid_(0),
        msg_handler_(nullptr),
        open_handler_(nullptr),
        close_handler_(nullptr) {
    namespace p = std::placeholders;
    server_.set_access_channels(websocketpp::log::alevel::none);
    server_.set_open_handler(bind(&ws_server_impl::on_open, this, p::_1));
    server_.set_close_handler(bind(&ws_server_impl::on_close, this, p::_1));
    server_.set_message_handler(
        bind(&ws_server_impl::on_msg, this, p::_1, p::_2));
  }

  void set_msg_handler(msg_handler handler) {
    msg_handler_ = std::move(handler);
  }

  void set_open_handler(sid_handler handler) {
    open_handler_ = std::move(handler);
  }

  void set_close_handler(sid_handler handler) {
    close_handler_ = std::move(handler);
  }

  void listen(std::string const& host, std::string const& port) {
    server_.init_asio(&ios_);
    server_.listen(host, port);
    server_.start_accept();
  }

  void send(json11::Json const& message, sid session) {
    ios_.post([this, session, message]() {
      auto sid_it = sid_con_map_.find(session);
      if (sid_it == end(sid_con_map_)) {
        return;
      }

      error_code send_ec;
      server_.send(sid_it->second, message.dump(), TEXT, send_ec);
    });
  }

  void stop() { server_.stop(); }

  void on_open(connection_hdl hdl) {
    sid_con_map_.insert({next_sid_, hdl});
    con_sid_map_.insert({hdl, next_sid_});

    if (open_handler_) {
      open_handler_(next_sid_);
    }

    ++next_sid_;
  }

  void on_close(connection_hdl hdl) {
    auto con_it = con_sid_map_.find(hdl);
    if (con_it == end(con_sid_map_)) {
      return;
    }
    con_sid_map_.erase(con_it);

    auto sid = con_it->second;
    auto sid_it = sid_con_map_.find(sid);
    if (sid_it == end(sid_con_map_)) {
      return;
    }
    sid_con_map_.erase(sid_it);

    if (close_handler_) {
      close_handler_(sid);
    }
  }

  void on_msg(connection_hdl con, asio_ws_server::message_ptr msg) {
    if (!msg_handler_) {
      return;
    }

    auto con_it = con_sid_map_.find(con);
    if (con_it == end(con_sid_map_)) {
      return;
    }

    std::string parse_error;
    auto json = json11::Json::parse(msg->get_payload(), parse_error);
    if (parse_error.empty()) {
      send(msg_handler_(json, con_it->second), con_it->second);
    } else {
      std::cerr << "parser error for message:\n";
      std::cout << msg->get_payload();
      std::cout << "\n";
      std::cout << parse_error << "\n";
    }
  }

  asio_ws_server server_;
  boost::asio::io_service& ios_;
  sid next_sid_;
  std::map<sid, connection_hdl> sid_con_map_;
  std::map<connection_hdl, sid, std::owner_less<connection_hdl>> con_sid_map_;

  msg_handler msg_handler_;
  sid_handler open_handler_;
  sid_handler close_handler_;
};

ws_server::~ws_server() {}

ws_server::ws_server(boost::asio::io_service& ios)
    : impl_(new ws_server_impl(ios)) {}

void ws_server::on_msg(msg_handler handler) {
  impl_->set_msg_handler(std::move(handler));
}

void ws_server::on_open(sid_handler handler) {
  impl_->set_open_handler(std::move(handler));
}

void ws_server::on_close(sid_handler handler) {
  impl_->set_close_handler(std::move(handler));
}

void ws_server::listen(std::string const& host, std::string const& port) {
  impl_->listen(host, port);
}

void ws_server::stop() { impl_->stop(); }

void ws_server::send(json11::Json const& msg, sid s) { impl_->send(msg, s); }

}  // namespace webservice
}  // namespace motis
