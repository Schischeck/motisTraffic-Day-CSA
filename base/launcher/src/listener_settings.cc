#include "motis/launcher/listener_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define HOST "listen.host"
#define PORT "listen.port"
#define API_KEY "listen.api_key"

namespace po = boost::program_options;

namespace motis {
namespace launcher {

listener_settings::listener_settings(std::string default_host,
                                     std::string default_port,
                                     std::string api_key)
    : host(default_host), port(default_port), api_key(api_key) {}

boost::program_options::options_description listener_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()(HOST, po::value<std::string>(&host)->default_value(host),
                     "host to listen on (e.g. 0.0.0.0 or 127.0.0.1)");
  desc.add_options()(PORT, po::value<std::string>(&port)->default_value(port),
                     "port to listen on (e.g. 8080)");
  desc.add_options()(API_KEY,
                     po::value<std::string>(&api_key)->default_value(api_key),
                     "API key for requests, leave empty to skip auth");
  return desc;
}

void listener_settings::print(std::ostream& out) const {
  out << "  " << HOST << ": " << host << "\n"
      << "  " << PORT << ": " << port << "\n"
      << "  " << API_KEY << ": " << api_key << "\n";
}

}  // namespace launcher
}  // namespace motis
