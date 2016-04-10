#include "motis/launcher/listener_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define LISTEN_WS "listen.ws"
#define LISTEN_HTTP "listen.http"
#define LISTEN_TCP "listen.tcp"
#define WS_HOST "listen.ws_host"
#define WS_PORT "listen.ws_port"
#define TCP_HOST "listen.tcp_host"
#define TCP_PORT "listen.tcp_port"
#define HTTP_HOST "listen.http_host"
#define HTTP_PORT "listen.http_port"
#define API_KEY "listen.api_key"

namespace po = boost::program_options;

namespace motis {
namespace launcher {

listener_settings::listener_settings(bool listen_ws, bool listen_http,
                                     bool listen_tcp, std::string ws_host,
                                     std::string ws_port, std::string http_host,
                                     std::string http_port,
                                     std::string tcp_host, std::string tcp_port,
                                     std::string api_key)
    : listen_ws(listen_ws),
      listen_http(listen_http),
      listen_tcp(listen_tcp),
      ws_host(std::move(ws_host)),
      ws_port(std::move(ws_port)),
      http_host(std::move(http_host)),
      http_port(std::move(http_port)),
      tcp_host(std::move(tcp_host)),
      tcp_port(std::move(tcp_port)),
      api_key(std::move(api_key)) {}

boost::program_options::options_description listener_settings::desc() {
  po::options_description desc("Listener Options");
  // clang-format off
  desc.add_options()
      (LISTEN_WS,
       po::value<bool>(&listen_ws)->default_value(listen_ws),
       "enable websocket listener")
      (LISTEN_HTTP,
       po::value<bool>(&listen_http)->default_value(listen_http),
       "enable http listener")
      (LISTEN_TCP,
       po::value<bool>(&listen_tcp)->default_value(listen_tcp),
       "enable tcp listener")
      (WS_HOST,
       po::value<std::string>(&ws_host)->default_value(ws_host),
       "websocket listener host")
      (WS_PORT,
       po::value<std::string>(&ws_port)->default_value(ws_port),
       "websocket listener port")
      (HTTP_HOST,
       po::value<std::string>(&http_host)->default_value(http_host),
       "http listener host")
      (HTTP_PORT,
       po::value<std::string>(&http_port)->default_value(http_port),
       "http listener port")
      (TCP_HOST,
       po::value<std::string>(&tcp_host)->default_value(tcp_host),
       "tcp listener host")
      (TCP_PORT,
       po::value<std::string>(&tcp_port)->default_value(tcp_port),
       "tcp listener port")
      (API_KEY,
       po::value<std::string>(&api_key)->default_value(api_key),
       "API key for requests, leave empty to skip auth");
  // clang-format on
  return desc;
}

void listener_settings::print(std::ostream& out) const {
  out << "  " << LISTEN_WS << ": " << listen_ws << "\n"
      << "  " << LISTEN_HTTP << ": " << listen_http << "\n"
      << "  " << LISTEN_TCP << ": " << listen_tcp << "\n"
      << "  " << WS_HOST << ": " << ws_host << "\n"
      << "  " << WS_PORT << ": " << ws_port << "\n"
      << "  " << HTTP_HOST << ": " << http_host << "\n"
      << "  " << HTTP_PORT << ": " << http_port << "\n"
      << "  " << TCP_HOST << ": " << tcp_host << "\n"
      << "  " << TCP_PORT << ": " << tcp_port << "\n"
      << "  " << API_KEY << ": " << api_key << "\n";
}

}  // namespace launcher
}  // namespace motis
