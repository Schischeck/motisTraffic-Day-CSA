#include "motis/cc/cc.h"

#include "motis/core/journey/message_to_journeys.h"

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

msg_ptr cc::check_journey(msg_ptr const& msg) const {
  auto c = motis_content(Connection, msg);

  for (auto const& s : *c->stops()) {
  }

  return msg;
}

}  // namespace cc
}  // namespace motis
