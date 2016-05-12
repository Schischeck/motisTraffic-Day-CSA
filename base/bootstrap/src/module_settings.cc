#include "motis/bootstrap/module_settings.h"

#include <ostream>

#include "motis/core/common/util.h"

#define MODULES "modules"

namespace motis {
namespace bootstrap {

namespace po = boost::program_options;

module_settings::module_settings(std::vector<std::string> modules)
    : modules_(std::move(modules)) {}

po::options_description module_settings::desc() {
  po::options_description desc("Module Settings");
  // clang-format off
  desc.add_options()
      (MODULES, po::value<std::vector<std::string>>(&modules_)
           ->default_value(modules_)->multitoken(),
       "List of modules to load");
  // clang-format on
  return desc;
}

void module_settings::print(std::ostream& out) const {
  out << "  " << MODULES << ": " << modules_;
}

}  // namespace bootstrap
}  // namespace motis
