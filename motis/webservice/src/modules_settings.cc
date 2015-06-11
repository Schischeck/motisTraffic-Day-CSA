#include "motis/webservice/modules_settings.h"

#include <ostream>

#define MODULES_PATH "modules_path"

namespace motis {
namespace webservice {

namespace po = boost::program_options;

modules_settings::modules_settings(std::string default_modules_path)
    : modules_path(std::move(default_modules_path)) {}

po::options_description modules_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()(MODULES_PATH, po::value<std::string>(&modules_path)
                                       ->default_value(modules_path),
                     "TD Dataset root");
  return desc;
}

void modules_settings::print(std::ostream& out) const {
  out << "  " << MODULES_PATH << ": " << modules_path;
}

}  // namespace webservice
}  // namespace motis
