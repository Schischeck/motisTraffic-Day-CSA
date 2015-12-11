#include "motis/launcher/http_server.h"

#include <functional>

#include "net/http/server/server.hpp"
#include "net/http/server/query_router.hpp"

#include "motis/module/message.h"
#include "motis/loader/util.h"

using namespace net::http::server;
using namespace motis::module;
namespace p = std::placeholders;

namespace motis {
namespace launcher {

HTTPMethod translate_method_string(std::string const& s) {
  if (s == "GET") {
    return HTTPMethod_GET;
  } else if (s == "POST") {
    return HTTPMethod_POST;
  } else if (s == "DELETE") {
    return HTTPMethod_DELETE;
  } else if (s == "PUT") {
    return HTTPMethod_PUT;
  } else if (s == "OPTIONS") {
    return HTTPMethod_OPTIONS;
  }
  return HTTPMethod_GET;
}

struct http_server::impl {
  impl(boost::asio::io_service& ios, receiver& recvr)
      : receiver_(recvr), server_(ios) {}

  void listen(std::string const& host, std::string const& port) {
    router_.route("*", ".*", std::ref(*this));
    server_.listen(host, port, router_);
  }

  void operator()(net::http::server::route_request const& req,
                  net::http::server::callback cb) {
    MessageCreator fbb;
    fbb.CreateAndFinish(
        MsgContent_HTTPRequest,
        CreateHTTPRequest(
            fbb, translate_method_string(req.method), fbb.CreateString(req.uri),
            fbb.CreateVector(loader::transform_to_vec(
                begin(req.headers), end(req.headers),
                [&](header const& h) {
                  return CreateHTTPHeader(fbb, fbb.CreateString(h.name),
                                          fbb.CreateString(h.value));
                })),
            fbb.CreateString(req.content))
            .Union());
    receiver_.on_msg(make_msg(fbb), 0,
                     std::bind(&impl::on_response, this, cb, p::_1, p::_2));
  }

  void on_response(net::http::server::callback cb, msg_ptr msg,
                   boost::system::error_code ec) {
    reply rep;
    if (!ec && msg && msg->content_type() == MsgContent_HTTPResponse) {
      auto http_res = msg->content<HTTPResponse const*>();
      rep.status = http_res->status() == HTTPStatus_OK
                       ? reply::ok
                       : reply::internal_server_error;
      rep.content = http_res->content()->str();
      for (auto const& h : *http_res->headers()) {
        rep.headers.push_back(header(h->name()->str(), h->value()->str()));
      }
    } else if (!ec) {
      rep = reply::stock_reply(reply::ok);
    } else {
      rep = reply::stock_reply(reply::internal_server_error);
      rep.content = ec.message();
    }
    return cb(rep);
  }

private:
  receiver& receiver_;
  query_router router_;
  server server_;
};

http_server::http_server(boost::asio::io_service& ios, receiver& recvr)
    : impl_(new impl(ios, recvr)) {}

http_server::~http_server() {}

void http_server::listen(std::string const& host, std::string const& port) {
  impl_->listen(host, port);
}

}  // namespace launcher
}  // namespace motis
