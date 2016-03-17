#include "motis/ris/ris.h"

#include "motis/core/common/util.h"
#include "motis/ris/mode/live_mode.h"
#include "motis/ris/mode/simulation_mode.h"

#define MODE "ris.mode"
#define UPDATE_INTERVAL "ris.update_interval"
#define ZIP_FOLDER "ris.zip_folder"
#define MAX_DAYS "ris.max_days"

#define SIM_INIT_START "ris.sim_init_start"
#define SIM_INIT_END "ris.sim_init_end"

#define MODE_LIVE "default"
#define MODE_SIMULATION "simulation"

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
  } else {
    throw std::runtime_error("unknown mode of operation in ris module.");
  }
  return in;
}

std::ostream& operator<<(std::ostream& out, mode_t const& mode) {
  switch (mode) {
    case mode_t::LIVE: out << MODE_LIVE; break;
    case mode_t::SIMULATION: out << MODE_SIMULATION; break;
    default: out << "unknown"; break;
  }
  return out;
}

ris::ris()
    : mode_(mode_t::SIMULATION),
      update_interval_(10),
      zip_folder_("ris"),
      max_days_(-1),
      sim_init_start_(0),
      sim_init_end_(0) {}

po::options_description ris::desc() {
  po::options_description desc("RIS Module");
  // clang-format off
  desc.add_options()
      (MODE,
       po::value<mode_t>(&mode_)->default_value(mode_),
       "Mode of operation. Valid choices:\n"
       MODE_LIVE " = production style operation\n"
       MODE_SIMULATION " = init db with fs, fwd via msg")
      (UPDATE_INTERVAL,
       po::value<int>(&update_interval_)->default_value(update_interval_),
       "update interval in seconds")
      (ZIP_FOLDER,
       po::value<std::string>(&zip_folder_)->default_value(zip_folder_),
       "folder containing RISML ZIPs")
      (MAX_DAYS,
       po::value<int>(&max_days_)->default_value(max_days_),
       "periodically delete messages older than n days (-1 = infinite)")
      (SIM_INIT_START,
       po::value<std::time_t>(&sim_init_start_)->default_value(sim_init_start_),
       "lower bound simulation clock init")
      (SIM_INIT_END,
       po::value<std::time_t>(&sim_init_end_)->default_value(sim_init_end_),
       "'forward' the simulation clock (expects Unix timestamp)");
  // clang-format on
  return desc;
}

void ris::print(std::ostream& out) const {
  out << "  " << MODE << ": " << mode_ << "\n"
      << "  " << UPDATE_INTERVAL << ": " << update_interval_ << "\n"
      << "  " << ZIP_FOLDER << ": " << zip_folder_ << "\n"
      << "  " << MAX_DAYS << ": " << max_days_ << "\n"
      << "  " << SIM_INIT_START << ": " << sim_init_start_ << "\n"
      << "  " << SIM_INIT_END << ": " << sim_init_end_;
}

void ris::init() {
  switch (mode_) {
    case mode_t::LIVE:  //
      active_mode_ = make_unique<live_mode>(this);
      break;
    case mode_t::SIMULATION:
      active_mode_ = make_unique<simulation_mode>(this);
      break;
    default: assert(false);
  }
}

void ris::init_async() { active_mode_->init_async(); }

void ris::on_msg(msg_ptr msg, sid s, callback cb) {
  active_mode_->on_msg(msg, s, cb);
}

}  // namespace ris
}  // namespace motis
