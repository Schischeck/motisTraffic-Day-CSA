#include "./test_spec_test.h"

#include "motis/loader/hrd/hrd_parser.h"
#include "motis/loader/hrd/parser/service_parser.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers64;

std::vector<specification> test_spec::get_specs() {
  std::vector<specification> specs;
  parse_specification(lf_,
                      [&specs](specification spec) { specs.push_back(spec); });
  return specs;
}

std::vector<hrd_service> test_spec::get_hrd_services() {
  std::vector<hrd_service> services;
  parse_specification(lf_, [&services](specification const& spec) {
    services.emplace_back(spec);
  });
  return services;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
