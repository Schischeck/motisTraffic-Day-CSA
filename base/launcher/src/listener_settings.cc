#include "motis/launcher/listener_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define LISTEN_WS "listen.ws"
#define LISTEN_HTTP "listen.http"
#define LISTEN_TCP "listen.tcp"
#define WS_HOST "listen.ws_host"
#define WS_PORT "listen.ws_port"
#define WS_BINARY "listen.ws_binary"
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
                                     std::string ws_port, bool ws_binary,
                                     std::string http_host,
                                     std::string http_port,
                                     std::string tcp_host, std::string tcp_port,
                                     std::string api_key)
    : listen_ws_(listen_ws),
      listen_http_(listen_http),
      listen_tcp_(listen_tcp),
      ws_host_(std::move(ws_host)),
      ws_port_(std::move(ws_port)),
      ws_binary_(ws_binary),
      http_host_(std::move(http_host)),
      http_port_(std::move(http_port)),
      tcp_host_(std::move(tcp_host)),
      tcp_port_(std::move(tcp_port)),
      api_key_(std::move(api_key)) {}

boost::program_options::options_description listener_settings::desc() {
  po::options_description desc("Listener Options");
  // clang-format off
  desc.add_options()
      (LISTEN_WS,
       po::value<bool>(&listen_ws_)->default_value(listen_ws_),
       "enable websocket listener")
      (LISTEN_HTTP,
       po::value<bool>(&listen_http_)->default_value(listen_http_),
       "enable http listener")
      (LISTEN_TCP,
       po::value<bool>(&listen_tcp_)->default_value(listen_tcp_),
       "enable tcp listener")
      (WS_HOST,
       po::value<std::string>(&ws_host_)->default_value(ws_host_),
       "websocket listener host")
      (WS_PORT,
       po::value<std::string>(&ws_port_)->default_value(ws_port_),
       "websocket listener port")
      (WS_BINARY,
       po::value<bool>(&ws_binary_)->default_value(ws_binary_),
       "websocket binary mode (flatbuffers+snappy)")
      (HTTP_HOST,
       po::value<std::string>(&http_host_)->default_value(http_host_),
       "http listener host")
      (HTTP_PORT,
       po::value<std::string>(&http_port_)->default_value(http_port_),
       "http listener port")
      (TCP_HOST,
       po::value<std::string>(&tcp_host_)->default_value(tcp_host_),
       "tcp listener host")
      (TCP_PORT,
       po::value<std::string>(&tcp_port_)->default_value(tcp_port_),
       "tcp listener port")
      (API_KEY,
       po::value<std::string>(&api_key_)->default_value(api_key_),
       "API key for requests, leave empty to skip auth");
  // clang-format on
  return desc;
}

void listener_settings::print(std::ostream& out) const {
  out << "  " << LISTEN_WS << ": " << listen_ws_ << "\n"
      << "  " << LISTEN_HTTP << ": " << listen_http_ << "\n"
      << "  " << LISTEN_TCP << ": " << listen_tcp_ << "\n"
      << "  " << WS_HOST << ": " << ws_host_ << "\n"
      << "  " << WS_PORT << ": " << ws_port_ << "\n"
      << "  " << WS_BINARY << ": " << ws_binary_ << "\n"
      << "  " << HTTP_HOST << ": " << http_host_ << "\n"
      << "  " << HTTP_PORT << ": " << http_port_ << "\n"
      << "  " << TCP_HOST << ": " << tcp_host_ << "\n"
      << "  " << TCP_PORT << ": " << tcp_port_ << "\n"
      << "  " << API_KEY << ": " << api_key_;
}

}  // namespace launcher
}  // namespace motis
