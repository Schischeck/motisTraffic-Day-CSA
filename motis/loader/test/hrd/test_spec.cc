#include "test_spec.h"

#include "motis/loader/parsers/hrd/hrd_parser.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;
namespace fs = boost::filesystem;

std::vector<specification> test_spec::get_specs() {
  std::vector<specification> specs;
  parse_specification(lf_, [&specs](specification spec) { specs.push_back(spec); });
  return specs;
}

std::vector<hrd_service> test_spec::get_hrd_services() {
  std::vector<hrd_service> services;
  parse_specification(lf_, [&services](specification const& spec) {
    services.push_back(hrd_service(spec));
  });
  return services;
}

}  // hrd
}  // loader
}  // motis
