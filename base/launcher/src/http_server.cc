#include "motis/launcher/http_server.h"

#include <functional>
#include <system_error>

#include "boost/algorithm/string/predicate.hpp"

#include "net/http/server/query_router.hpp"
#include "net/http/server/server.hpp"

#include "motis/module/message.h"
#include "motis/loader/util.h"

using namespace net::http::server;
using namespace motis::module;
namespace p = std::placeholders;
namespace srv = net::http::server;

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
      : ios_(ios), receiver_(recvr), server_(ios) {}

  void listen(std::string const& host, std::string const& port) {
    router_.route("*", ".*", std::ref(*this));
    server_.listen(host, port, router_);
  }

  void stop() { server_.stop(); }

  void operator()(srv::route_request const& req, srv::callback cb) {
    try {
      if (req.method == "GET") {
        return handle_get(req, cb);
      } else if (req.method == "POST") {
        return handle_post(req, cb);
      } else if (req.method == "OPTIONS") {
        return handle_options(req, cb);
      } else {
        return cb(reply::stock_reply(reply::not_found));
      }
    } catch (std::system_error const& e) {
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

  void handle_get(srv::route_request const& req, srv::callback& cb) {
    return receiver_.on_msg(
        make_no_msg(get_path(req.uri)),
        ios_.wrap(std::bind(&impl::on_response, this, cb, p::_1, p::_2)));
  }

  void handle_post(srv::route_request const& req, srv::callback& cb) {
    if (has_header(req, "Content-Type", "application/json")) {
      return receiver_.on_msg(
          make_msg(req.content),
          ios_.wrap(std::bind(&impl::on_response, this, cb, p::_1, p::_2)));
    } else {
      message_creator fbb;
      fbb.create_and_finish(
          MsgContent_HTTPRequest,
          CreateHTTPRequest(fbb, translate_method_string(req.method),
                            fbb.CreateString(get_path(req.uri)),
                            fbb.CreateVector(loader::transform_to_vec(
                                begin(req.headers), end(req.headers),
                                [&](header const& h) {
                                  return CreateHTTPHeader(
                                      fbb, fbb.CreateString(h.name),
                                      fbb.CreateString(h.value));
                                })),
                            fbb.CreateString(req.content))
              .Union(),
          get_path(req.uri));
      return receiver_.on_msg(
          make_msg(fbb),
          ios_.wrap(std::bind(&impl::on_response, this, cb, p::_1, p::_2)));
    }
  }

  void handle_options(srv::route_request const&, srv::callback& cb) {
    reply rep;
    rep.status = reply::status_type::ok;
    add_cors_headers(rep);
    cb(rep);
  }

  void add_cors_headers(reply& rep) const {
    rep.headers.emplace_back("Access-Control-Allow-Origin", "*");
    rep.headers.emplace_back(
        "Access-Control-Allow-Headers",
        "X-Requested-With, Content-Type, Accept, Authorization");
    rep.headers.emplace_back("Access-Control-Allow-Methods",
                             "GET, POST, PUT, DELETE, OPTIONS, HEAD");
  }

  std::string get_path(std::string const& uri) {
    auto pos = uri.find('?');
    if (pos != std::string::npos) {
      return uri.substr(0, pos);
    }
    return uri;
  }

  bool has_header(srv::route_request const& req, char const* k, char const* v) {
    auto it =
        std::find_if(begin(req.headers), end(req.headers),
                     [&k](auto&& h) { return boost::iequals(h.name, k); });
    return it != end(req.headers) && it->value.find(v) != std::string::npos;
  }

  void on_response(srv::callback cb, msg_ptr msg, std::error_code ec) {
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
      add_cors_headers(rep);
    } catch (...) {
      // reply is already set to internal_server_error
    }
    return cb(rep);
  }

private:
  boost::asio::io_service& ios_;
  receiver& receiver_;
  query_router router_;
  server server_;
};

http_server::http_server(boost::asio::io_service& ios, receiver& recvr)
    : impl_(new impl(ios, recvr)) {}

http_server::~http_server() = default;

void http_server::listen(std::string const& host, std::string const& port) {
  impl_->listen(host, port);
}

void http_server::stop() { impl_->stop(); }

}  // namespace launcher
}  // namespace motis
