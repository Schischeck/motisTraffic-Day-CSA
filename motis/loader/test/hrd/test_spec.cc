#include "test_spec.h"

#include "motis/loader/parsers/hrd/service/service_parser.h"

namespace motis {
namespace loader {
namespace hrd {

namespace fs = boost::filesystem;

std::vector<hrd_service> test_spec::get_services() {
  std::vector<hrd_service> services;
  parse_services(lf_, [&services](specification const& spec) {
    services.push_back(hrd_service(spec));
  });
  return services;
}

std::vector<specification> test_spec::get_specs() {
  std::vector<specification> specs;
  parse_services(lf_, [&specs](specification spec) { specs.push_back(spec); });
  return specs;
}

}  // hrd
}  // loader
}  // motis
