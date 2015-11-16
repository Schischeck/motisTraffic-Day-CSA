#include "motis/launcher/launcher_settings.h"

#include <ostream>

#include "boost/lexical_cast.hpp"
#include "boost/date_time/local_time/local_time.hpp"

#include "motis/core/common/date_util.h"

#define MODE "mode"
#define MODULES "modules"
#define MODE_BATCH "batch"
#define MODE_SERVER "server"
#define MODE_TEST "test"

namespace std {
template <typename T>
ostream& operator<<(ostream& out, vector<T> const& v) {
  for (auto const& el : v) {
    out << el << " ";
  }
  return out;
}
}  // namespace std

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

launcher_settings::launcher_settings(motis_mode_t m,
                                     std::vector<std::string> modules)
    : mode(m), modules(std::move(modules)) {}

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
      (MODULES, po::value<std::vector<std::string>>(&modules)
          ->default_value(modules)->multitoken(),
       "List of modules to load");
  // clang-format on
  return desc;
}

void launcher_settings::print(std::ostream& out) const {
  out << "  " << MODE << ": " << mode << "\n"
      << "  " << MODULES << ": " << modules;
}

}  // namespace launcher
}  // namespace motis
