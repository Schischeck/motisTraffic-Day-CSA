#include "execution/callback_settings.h"

#include <ostream>

#include "boost/program_options.hpp"
#include "boost/io/ios_state.hpp"

#define ENABLED "callback.enabled"
#define HOST "callback.host"
#define PORT "callback.port"
#define INSTANCE_ID "callback.id"

namespace po = boost::program_options;

namespace td {
namespace execution {

callback_settings::callback_settings(bool default_enabled,
                                     std::string default_host,
                                     std::string default_port,
                                     std::string default_id)
    : enabled(default_enabled),
      host(default_host),
      port(default_port),
      id(default_id) {
}

boost::program_options::options_description callback_settings::desc() {
  po::options_description desc("Callback Options");
  desc.add_options()
    (ENABLED,
        po::value<bool>(&enabled)->default_value(enabled),
        "should this instance call back after startup?")
    (HOST,
        po::value<std::string>(&host)->default_value(host),
        "callback host")
    (PORT,
        po::value<std::string>(&port)->default_value(port),
        "callback port")
    (INSTANCE_ID,
        po::value<std::string>(&id)->default_value(id),
        "callback instance id");
  return desc;
}

void callback_settings::print(std::ostream& out) const {
  boost::io::ios_all_saver guard(out);
  out << "  " << ENABLED << ": " << std::boolalpha << enabled << "\n"
      << "  " << HOST << ": " << host << "\n"
      << "  " << PORT << ": " << port << "\n"
      << "  " << INSTANCE_ID << ": " << id << "\n";
}

}  // namespace execution
}  // namespace td
