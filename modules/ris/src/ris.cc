#include "motis/ris/ris.h"

#include <memory>

#include "conf/simple_config_param.h"

// #include "motis/ris/mode/live_mode.h"
#include "motis/ris/mode/simulation_mode.h"
#include "motis/ris/mode/test_mode.h"

#define MODE_LIVE "live"
#define MODE_SIMULATION "simulation"
#define MODE_TEST "test"

using namespace motis::module;
using namespace motis::ris::mode;

namespace motis {
namespace ris {

std::istream& operator>>(std::istream& in, mode_t& mode) {
  std::string token;
  in >> token;
  if (token == MODE_LIVE) {
    mode = mode_t::LIVE;
  } else if (token == MODE_SIMULATION) {
    mode = mode_t::SIMULATION;
  } else if (token == MODE_TEST) {
    mode = mode_t::TEST;
  } else {
    throw std::runtime_error("unknown mode of operation in ris module.");
  }
  return in;
}

std::ostream& operator<<(std::ostream& out, mode_t const& mode) {
  switch (mode) {
    case mode_t::LIVE: out << MODE_LIVE; break;
    case mode_t::SIMULATION: out << MODE_SIMULATION; break;
    case mode_t::TEST: out << MODE_TEST; break;
    default: out << "unknown"; break;
  }
  return out;
}

ris::ris() : module("RIS Options", "ris") {
  template_param(conf_.mode_, mode_t::SIMULATION, "mode",
                 "Mode of operation. Valid choices:\n"  //
                 MODE_LIVE " = production style operation\n"  //
                 MODE_SIMULATION " = init db with fs, fwd via msg\n"  //
                 MODE_TEST " = init with a folder of xml files\n");
  string_param(conf_.input_folder_, "ris", "input_folder",
               "folder containing RISML ZIPs");
  string_param(conf_.database_file_, "ris.sqlite3", "database_file",
               "SQLite3 database for parsed message caching");
  int_param(conf_.update_interval_, 10, "update_interval",
            "update interval in seconds");
  int_param(conf_.max_days_, -1, "max_days",
            "periodically delete messages older than n days (-1 = infinite)");
  template_param(conf_.sim_init_time_, 0l, "sim_init_time",
                 "'forward' the simulation clock (expects Unix timestamp)");
}

ris::~ris() = default;

void ris::init(registry& r) {
  switch (conf_.mode_) {
    case mode_t::LIVE:  //
      // active_mode_ = std::make_unique<live_mode>(conf_);
      break;
    case mode_t::SIMULATION:
      active_mode_ = std::make_unique<simulation_mode>(&conf_);
      break;
    case mode_t::TEST:
      active_mode_ = std::make_unique<test_mode>(&conf_);
      break;
    default: assert(false);
  }

  active_mode_->init(r);
}

}  // namespace ris
}  // namespace motis
