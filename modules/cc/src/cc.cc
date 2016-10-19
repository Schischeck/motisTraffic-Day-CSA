#include "motis/cc/cc.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;
using namespace motis::module;

namespace motis {
namespace cc {

po::options_description cc::desc() {
  return po::options_description("RT Module");
}

void cc::init(motis::module::registry& reg) {
  namespace p = std::placeholders;
  reg.subscribe("/cc/check_journey",
                std::bind(&cc::check_journey, this, p::_1));
}

msg_ptr cc::check_journey(msg_ptr const&) const { return nullptr; }

}  // namespace cc
}  // namespace motis
