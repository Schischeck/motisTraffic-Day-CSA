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
        std::bind(&ws_server_impl::on_msg, this, p::_1, p::_2));
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
    flatbuffers::FlatBufferBuilder b;
    b.Finish(CreateMessage(
        b, MsgContent_MotisError,
        CreateMotisError(b, e.value(), b.CreateString(e.category().name()),
                         b.CreateString(e.message()))
            .Union()));
    send(make_msg(b), session, request_id);
  }

  void send_success(sid session, int request_id) {
    flatbuffers::FlatBufferBuilder b;
    b.Finish(CreateMessage(b, MsgContent_MotisSuccess,
                           CreateMotisSuccess(b).Union()));
    send(make_msg(b), session, request_id);
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

    auto sid = con_it->second;
    con_sid_map_.erase(con_it);

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

    msg_ptr req_msg;
    try {
      req_msg = make_msg(msg->get_payload());
    } catch (...) {
      return;
    }

    auto session = con_it->second;
    try {
      msg_handler_(
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

void ws_server::send(msg_ptr const& msg, sid s) { impl_->send(msg, s, 0); }

}  // namespace launcher
}  // namespace motis
