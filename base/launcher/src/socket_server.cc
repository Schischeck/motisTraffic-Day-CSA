#include "motis/launcher/socket_server.h"

#include <functional>

#include "snappy.h"

#include "net/tcp_server.h"

#include "motis/module/message.h"
#include "motis/loader/util.h"

using net::tcp_server;
using namespace motis::module;
namespace p = std::placeholders;

namespace motis {
namespace launcher {

struct socket_server::impl {
  impl(boost::asio::io_service& ios, receiver& recvr)
      : receiver_(recvr), server_(net::make_server(ios)) {}

  void listen(std::string const& host, std::string const& port) {
    server_->listen(host, port, std::bind(&impl::receive, this, p::_1, p::_2));
  }

  void stop() { server_->stop(); }

  void receive(std::string const& request, net::handler_cb_fun cb) {
    snappy::Uncompress(static_cast<char const*>(request.data()), request.size(),
                       &buf_);
    auto req_msg = make_msg((void*)buf_.data(), buf_.size());
    receiver_.on_msg(req_msg, std::bind(&impl::reply, this, req_msg->id(), cb,
                                        p::_1, p::_2));
  }

  void reply(int id, net::handler_cb_fun cb, msg_ptr res,
             boost::system::error_code ec) {
    msg_ptr response;

    if (ec) {
      MessageCreator b;
      b.CreateAndFinish(
          MsgContent_MotisError,
          CreateMotisError(b, ec.value(), b.CreateString(ec.category().name()),
                           b.CreateString(ec.message()))
              .Union());
      response = make_msg(b);
    } else if (res) {
      response = res;
    } else {
      MessageCreator b;
      b.CreateAndFinish(MsgContent_MotisSuccess, CreateMotisSuccess(b).Union());
      response = make_msg(b);
    }
    response->get()->mutate_id(id);

    std::string b;
    snappy::Compress(reinterpret_cast<char const*>(response->data()),
                     response->size(), &b);
    cb(std::ref(b), true);
  }

private:
  std::string buf_;
  receiver& receiver_;
  std::shared_ptr<tcp_server> server_;
};

socket_server::socket_server(boost::asio::io_service& ios, receiver& recvr)
    : impl_(new impl(ios, recvr)) {}

socket_server::~socket_server() {}

void socket_server::listen(std::string const& host, std::string const& port) {
  impl_->listen(host, port);
}

void socket_server::stop() { impl_->stop(); }

}  // namespace launcher
}  // namespace motis
