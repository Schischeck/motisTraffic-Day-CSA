#include "motis/launcher/http_server.h"

#include <functional>

#include "net/http/server/query_router.hpp"
#include "net/http/server/server.hpp"

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

  void stop() { server_.stop(); }

  void operator()(net::http::server::route_request const& req,
                  net::http::server::callback cb) {
    try {
      auto content_type_header_it = std::find_if(
          begin(req.headers), end(req.headers),
          [](header const& h) { return h.name == "Content-Type"; });
      if (req.method == "POST" &&  //
          content_type_header_it != end(req.headers) &&
          content_type_header_it->value.find("application/json") !=
              std::string::npos) {
        return receiver_.on_msg(
            make_msg(req.content),
            std::bind(&impl::on_response, this, cb, p::_1, p::_2));
      } else {
        MessageCreator fbb;
        fbb.CreateAndFinish(
            MsgContent_HTTPRequest,
            CreateHTTPRequest(fbb, translate_method_string(req.method),
                              fbb.CreateString(req.uri),
                              fbb.CreateVector(loader::transform_to_vec(
                                  begin(req.headers), end(req.headers),
                                  [&](header const& h) {
                                    return CreateHTTPHeader(
                                        fbb, fbb.CreateString(h.name),
                                        fbb.CreateString(h.value));
                                  })),
                              fbb.CreateString(req.content))
                .Union());
        return receiver_.on_msg(
            make_msg(fbb),
            std::bind(&impl::on_response, this, cb, p::_1, p::_2));
      }
    } catch (boost::system::system_error const& e) {
      reply rep = reply::stock_reply(reply::internal_server_error);
      rep.content = e.code().message();
      return cb(rep);
    } catch (std::exception const& e) {
      reply rep = reply::stock_reply(reply::internal_server_error);
      rep.content = e.what();
      return cb(rep);
    } catch (...) {
      return cb(reply::stock_reply(reply::internal_server_error));
    }
  }

  void on_response(net::http::server::callback cb, msg_ptr msg,
                   boost::system::error_code ec) {
    reply rep = reply::stock_reply(reply::internal_server_error);
    try {
      if (!ec && msg) {
        if (msg->get()->content_type() == MsgContent_HTTPResponse) {
          auto http_res = motis_content(HTTPResponse, msg);
          rep.status = http_res->status() == HTTPStatus_OK
                           ? reply::ok
                           : reply::internal_server_error;
          rep.content = http_res->content()->str();
          for (auto const& h : *http_res->headers()) {
            rep.headers.push_back(header(h->name()->str(), h->value()->str()));
          }
        } else {
          rep.content = msg->to_json();
          rep.status = reply::ok;
          rep.headers.push_back(header("Content-Type", "application/json"));
        }
      } else if (!ec) {
        rep = reply::stock_reply(reply::ok);
      } else {
        rep.content = ec.message();
      }
    } catch (...) {
      // reply is already set to internal_server_error
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

void http_server::stop() { impl_->stop(); }

}  // namespace launcher
}  // namespace motis
