#include "motis/launcher/launcher_settings.h"

#include <ostream>

#include "boost/lexical_cast.hpp"
#include "boost/date_time/local_time/local_time.hpp"

#include "motis/core/common/date_util.h"

#define MODE "mode"
#define MODE_BATCH "batch"
#define MODE_SERVER "server"
#define MODE_TEST "test"

namespace motis {
namespace launcher {

namespace po = boost::program_options;

std::istream& operator>>(std::istream& in,
                         launcher_settings::motis_mode_t& mode) {
  std::string token;
  in >> token;

  if (token == MODE_BATCH) {
    mode = launcher_settings::BATCH;
  } else if (token == MODE_SERVER) {
    mode = launcher_settings::SERVER;
  } else if (token == MODE_TEST) {
    mode = launcher_settings::TEST;
  }

  return in;
}

std::ostream& operator<<(std::ostream& out,
                         launcher_settings::motis_mode_t const& mode) {
  switch (mode) {
    case launcher_settings::BATCH: out << MODE_BATCH; break;
    case launcher_settings::SERVER: out << MODE_SERVER; break;
    case launcher_settings::TEST: out << MODE_TEST; break;
    default: out << "unknown"; break;
  }
  return out;
}

launcher_settings::launcher_settings(motis_mode_t m) : mode(m) {}

po::options_description launcher_settings::desc() {
  po::options_description desc("Mode Settings");
  // clang-format off
  desc.add_options()
      (MODE,
       po::value<motis_mode_t>(&mode)->default_value(mode),
       "Mode of operation. Valid choices:\n"
       MODE_BATCH " = read queries from batch file\n"
       MODE_SERVER " = listen for queries from network\n"
       MODE_TEST " = start server mode and exit after 1s")
      ("modules", po::value<std::vector<int>>()->multitoken(),
       "List of modules to load");
  // clang-format on
  return desc;
}

void launcher_settings::print(std::ostream& out) const {
  out << "  " << MODE << ": " << mode;
}

}  // namespace launcher
}  // namespace motis
