#include "motis/rt/rt.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

namespace motis {
namespace rt {

rt::rt() = default;

rt::~rt() = default;

po::options_description rt::desc() {
  return po::options_description("RT Module");
}

void rt::init(motis::module::registry& reg) {
  handler_ = std::make_unique<rt_handler>(synced_sched<RW>().sched());

  namespace p = std::placeholders;
  reg.subscribe("/ris/messages",
                std::bind(&rt_handler::update, handler_.get(), p::_1));
  reg.subscribe("/ris/system_time_changed",
                std::bind(&rt_handler::flush, handler_.get(), p::_1));
}

}  // namespace rt
}  // namespace motis
