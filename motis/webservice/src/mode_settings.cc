#include "motis/webservice/mode_settings.h"

#include <ostream>

#include "boost/lexical_cast.hpp"
#include "boost/date_time/local_time/local_time.hpp"

#include "motis/core/common/date_util.h"

#define MODE "mode"
#define MODE_BATCH "batch"
#define MODE_SERVER "server"
#define MODE_TEST "test"

namespace motis {
namespace webservice {

namespace po = boost::program_options;

std::istream& operator>>(std::istream& in,
                         motis::webservice::mode_settings::motis_mode_t& mode) {
  std::string token;
  in >> token;

  if (token == MODE_BATCH) {
    mode = motis::webservice::mode_settings::BATCH;
  } else if (token == MODE_SERVER) {
    mode = motis::webservice::mode_settings::SERVER;
  } else if (token == MODE_TEST) {
    mode = motis::webservice::mode_settings::TEST;
  }

  return in;
}

std::ostream& operator<<(
    std::ostream& out,
    motis::webservice::mode_settings::motis_mode_t const& mode) {
  switch (mode) {
    case motis::webservice::mode_settings::BATCH: out << MODE_BATCH; break;
    case motis::webservice::mode_settings::SERVER: out << MODE_SERVER; break;
    case motis::webservice::mode_settings::TEST: out << MODE_TEST; break;
    default: out << "unknown"; break;
  }
  return out;
}

mode_settings::mode_settings(motis_mode_t m) : mode(m) {}

po::options_description mode_settings::desc() {
  po::options_description desc("Mode Settings");
  // clang-format off
  desc.add_options()
      (MODE,
       po::value<motis_mode_t>(&mode)->default_value(mode),
       "Mode of operation. Valid choices:\n"
       MODE_BATCH " = read queries from batch file\n"
       MODE_SERVER " = listen for queries from network\n"
       MODE_TEST " = start server mode and exit after 1s");
  // clang-format on
  return desc;
}

void mode_settings::print(std::ostream& out) const {
  out << "  " << MODE << ": " << mode;
}

}  // namespace webservice
}  // namespace motis
