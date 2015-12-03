#include "motis/launcher/ws_server.h"

#include <memory>
#include <functional>

#include "boost/system/system_error.hpp"

#define WEBSOCKETPP_STRICT_MASKING
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> asio_ws_server;

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::error_code;
using websocketpp::frame::opcode::TEXT;

using namespace motis::module;

namespace motis {
namespace launcher {

struct ws_server::ws_server_impl {
  ws_server_impl(boost::asio::io_service& ios, receiver& receiver)
      : receiver_(receiver), ios_(ios), next_sid_(0) {
    namespace p = std::placeholders;
    server_.set_access_channels(websocketpp::log::alevel::none);
    server_.set_open_handler(bind(&ws_server_impl::on_open, this, p::_1));
    server_.set_close_handler(bind(&ws_server_impl::on_close, this, p::_1));
    server_.set_message_handler(
        std::bind(&ws_server_impl::on_msg, this, p::_1, p::_2));
  }

  void listen(std::string const& host, std::string const& port) {
    server_.init_asio(&ios_);
    server_.listen(host, port);
    server_.start_accept();
  }

  void send(msg_ptr const& msg, sid session, int request_id) {
    msg->msg_->mutate_id(request_id);

    if (!msg) {
      return;
    }

    ios_.post([this, session, msg]() {
      auto sid_it = sid_con_map_.find(session);
      if (sid_it == end(sid_con_map_)) {
        return;
      }

      error_code send_ec;
      server_.send(sid_it->second, msg->to_json(), TEXT, send_ec);
    });
  }

  void send_error(boost::system::error_code e, sid session, int request_id) {
    MessageCreator b;
    b.CreateAndFinish(
        MsgContent_MotisError,
        CreateMotisError(b, e.value(), b.CreateString(e.category().name()),
                         b.CreateString(e.message()))
            .Union());
    send(make_msg(b), session, request_id);
  }

  void send_success(sid session, int request_id) {
    MessageCreator b;
    b.CreateAndFinish(MsgContent_MotisSuccess, CreateMotisSuccess(b).Union());
    send(make_msg(b), session, request_id);
  }

  void stop() { server_.stop(); }

  void on_open(connection_hdl hdl) {
    sid_con_map_.insert({next_sid_, hdl});
    con_sid_map_.insert({hdl, next_sid_});
    receiver_.on_open((next_sid_));
    ++next_sid_;
  }

  void on_close(connection_hdl hdl) {
    auto con_it = con_sid_map_.find(hdl);
    if (con_it == end(con_sid_map_)) {
      return;
    }

    auto sid = con_it->second;
    con_sid_map_.erase(con_it);

    auto sid_it = sid_con_map_.find(sid);
    if (sid_it == end(sid_con_map_)) {
      return;
    }

    sid_con_map_.erase(sid_it);
    receiver_.on_close(sid);
  }

  void on_msg(connection_hdl con, asio_ws_server::message_ptr msg) {
    auto con_it = con_sid_map_.find(con);
    if (con_it == end(con_sid_map_)) {
      return;
    }
    auto session = con_it->second;

    msg_ptr req_msg;
    try {
      req_msg = make_msg(msg->get_payload());
    } catch (boost::system::system_error const& e) {
      send_error(e.code(), session, 0);
      return;
    } catch (...) {
      return;
    }

    try {
      receiver_.on_msg(
          req_msg, session,
          [this, session, req_msg](msg_ptr res, boost::system::error_code ec) {
            if (ec) {
              send_error(ec, session, req_msg->msg_->id());
            } else if (res) {
              send(res, session, req_msg->msg_->id());
            } else {
              send_success(session, req_msg->msg_->id());
            }
          });
    } catch (boost::system::system_error const& e) {
      send_error(e.code(), session, req_msg->msg_->id());
    } catch (...) {
      std::cout << "unknown error occured\n";
    }
  }

  motis::module::receiver& receiver_;

  asio_ws_server server_;
  boost::asio::io_service& ios_;

  sid next_sid_;
  std::map<sid, connection_hdl> sid_con_map_;
  std::map<connection_hdl, sid, std::owner_less<connection_hdl>> con_sid_map_;
};

ws_server::~ws_server() {}

ws_server::ws_server(boost::asio::io_service& ios,
                     motis::module::receiver& receiver)
    : impl_(new ws_server_impl(ios, receiver)) {}

void ws_server::listen(std::string const& host, std::string const& port) {
  impl_->listen(host, port);
}

void ws_server::stop() { impl_->stop(); }

void ws_server::send(msg_ptr const& msg, sid s) { impl_->send(msg, s, 0); }

}  // namespace launcher
}  // namespace motis
