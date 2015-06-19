#include "motis/webservice/modules_settings.h"

#include <ostream>

#define MODULES_PATH "modules_path"

namespace motis {
namespace webservice {

namespace po = boost::program_options;

modules_settings::modules_settings(std::string default_path)
    : path(std::move(default_path)) {}

po::options_description modules_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()(MODULES_PATH,
                     po::value<std::string>(&path)->default_value(path),
                     "TD Dataset root");
  return desc;
}

void modules_settings::print(std::ostream& out) const {
  out << "  " << MODULES_PATH << ": " << path;
}

}  // namespace webservice
}  // namespace motis
