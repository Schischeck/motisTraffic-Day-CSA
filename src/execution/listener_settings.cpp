#include "execution/listener_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define ENABLED "listen.enabled"
#define HOST "listen.host"
#define PORT "listen.port"

namespace po = boost::program_options;

namespace td {
namespace execution {

listener_settings::listener_settings(bool default_enable,
                                     std::string default_host,
                                     std::string default_port)
    : enabled(default_enable),
      host(default_host),
      port(default_port) {
}

boost::program_options::options_description listener_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()
    (ENABLED,
        po::value<bool>(&enabled)->default_value(enabled),
        "enable webservice (e.g. true or false)")
    (HOST,
        po::value<std::string>(&host)->default_value(host),
        "host to listen on (e.g. 0.0.0.0 or 127.0.0.1)")
    (PORT,
        po::value<std::string>(&port)->default_value(port),
        "port to listen on (e.g. 8080)");
  return desc;
}

void listener_settings::print(std::ostream& out) const {
  out << "  " << ENABLED << ": " << std::boolalpha << enabled << "\n"
      << "  " << HOST << ": " << host << "\n"
      << "  " << PORT << ": " << port;
}

}  // namespace execution
}  // namespace td
