#include "motis/launcher/socket_server.h"

#include <functional>

#include "snappy.h"

#include "net/tcp_server.h"

#include "utl/to_vec.h"

#include "motis/module/message.h"

using net::tcp_server;
using namespace motis::module;
namespace p = std::placeholders;

namespace motis {
namespace launcher {

struct socket_server::impl {
  impl(boost::asio::io_service& ios, receiver& recvr)
      : ios_(ios), receiver_(recvr), server_(net::make_server(ios)) {}

  void listen(std::string const& host, std::string const& port) {
    server_->listen(host, port, std::bind(&impl::receive, this, p::_1, p::_2));
  }

  void stop() { server_->stop(); }

  void receive(std::string const& request, net::handler_cb_fun cb) {
    snappy::Uncompress(static_cast<char const*>(request.data()), request.size(),
                       &buf_);
    auto req_msg =
        make_msg(reinterpret_cast<void const*>(buf_.data()), buf_.size());
    receiver_.on_msg(req_msg,
                     ios_.wrap(std::bind(&impl::reply, this, req_msg->id(), cb,
                                         p::_1, p::_2)));
  }

  void reply(int id, net::handler_cb_fun cb, msg_ptr res, std::error_code ec) {
    msg_ptr response;

    if (ec) {
      response = make_error_msg(ec);
    } else if (res) {
      response = res;
    } else {
      response = make_success_msg();
    }
    response->get()->mutate_id(id);

    std::string b;
    snappy::Compress(reinterpret_cast<char const*>(response->data()),
                     response->size(), &b);
    cb(std::ref(b), true);
  }

private:
  boost::asio::io_service& ios_;
  std::string buf_;
  receiver& receiver_;
  std::shared_ptr<tcp_server> server_;
};

socket_server::socket_server(boost::asio::io_service& ios, receiver& recvr)
    : impl_(new impl(ios, recvr)) {}

socket_server::~socket_server() = default;

void socket_server::listen(std::string const& host, std::string const& port) {
  impl_->listen(host, port);
}

void socket_server::stop() { impl_->stop(); }

}  // namespace launcher
}  // namespace motis
