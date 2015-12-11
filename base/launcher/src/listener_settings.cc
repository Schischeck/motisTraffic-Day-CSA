#include "motis/launcher/listener_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define LISTEN_WS "listen.ws"
#define LISTEN_HTTP "listen.http"
#define WS_HOST "listen.host"
#define WS_PORT "listen.port"
#define HTTP_HOST "listen.host"
#define HTTP_PORT "listen.port"
#define API_KEY "listen.api_key"

namespace po = boost::program_options;

namespace motis {
namespace launcher {

listener_settings::listener_settings(bool listen_ws, bool listen_http,
                                     std::string ws_host, std::string ws_port,
                                     std::string http_host,
                                     std::string http_port, std::string api_key)
    : listen_ws(listen_ws),
      listen_http(listen_http),
      ws_host(ws_host),
      ws_port(ws_port),
      http_host(http_host),
      http_port(http_port),
      api_key(api_key) {}

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
      (API_KEY,
       po::value<std::string>(&api_key)->default_value(api_key),
       "API key for requests, leave empty to skip auth");
  // clang-format on
  return desc;
}

void listener_settings::print(std::ostream& out) const {
  out << "  " << LISTEN_WS << ": " << listen_ws << "\n"
      << "  " << LISTEN_HTTP << ": " << listen_http << "\n"
      << "  " << WS_HOST << ": " << ws_host << "\n"
      << "  " << WS_PORT << ": " << ws_port << "\n"
      << "  " << HTTP_HOST << ": " << http_host << "\n"
      << "  " << HTTP_PORT << ": " << http_port << "\n"
      << "  " << API_KEY << ": " << api_key << "\n";
}

}  // namespace launcher
}  // namespace motis
