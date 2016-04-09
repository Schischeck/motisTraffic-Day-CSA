#include "motis/launcher/launcher_settings.h"

#include <ostream>

#include "boost/date_time/local_time/local_time.hpp"
#include "boost/lexical_cast.hpp"

#include "motis/core/common/date_util.h"
#include "motis/core/common/util.h"

#define MODE "mode"
#define MODULES "modules"
#define MODE_BATCH "batch"
#define MODE_SERVER "server"
#define MODE_TEST "test"
#define BATCH_INPUT_FILE "batch_input_file"
#define BATCH_OUTPUT_FILE "batch_output_file"
#define NUM_THREADS "num_threads"

namespace motis {
namespace launcher {

namespace po = boost::program_options;

std::istream& operator>>(std::istream& in,
                         launcher_settings::motis_mode_t& mode) {
  std::string token;
  in >> token;

  if (token == MODE_BATCH) {
    mode = launcher_settings::motis_mode_t::BATCH;
  } else if (token == MODE_SERVER) {
    mode = launcher_settings::motis_mode_t::SERVER;
  } else if (token == MODE_TEST) {
    mode = launcher_settings::motis_mode_t::TEST;
  }

  return in;
}

std::ostream& operator<<(std::ostream& out,
                         launcher_settings::motis_mode_t const& mode) {
  switch (mode) {
    case launcher_settings::motis_mode_t::BATCH: out << MODE_BATCH; break;
    case launcher_settings::motis_mode_t::SERVER: out << MODE_SERVER; break;
    case launcher_settings::motis_mode_t::TEST: out << MODE_TEST; break;
    default: out << "unknown"; break;
  }
  return out;
}

launcher_settings::launcher_settings(motis_mode_t m,
                                     std::vector<std::string> modules,
                                     std::string batch_input_file,
                                     std::string batch_output_file,
                                     int num_threads)
    : mode(m),
      modules(std::move(modules)),
      batch_input_file(batch_input_file),
      batch_output_file(batch_output_file),
      num_threads(num_threads) {}

po::options_description launcher_settings::desc() {
  po::options_description desc("Launcher Settings");
  // clang-format off
  desc.add_options()
      (MODE,
       po::value<motis_mode_t>(&mode)->default_value(mode),
       "Mode of operation:\n"
       MODE_BATCH " = inject batch file\n"
       MODE_SERVER " = network server\n"
       MODE_TEST " = exit after 1s")
      (MODULES, po::value<std::vector<std::string>>(&modules)
           ->default_value(modules)->multitoken(),
       "List of modules to load")
      (BATCH_INPUT_FILE,
       po::value<std::string>(&batch_input_file)
           ->default_value(batch_input_file))
      (BATCH_OUTPUT_FILE,
       po::value<std::string>(&batch_output_file)
           ->default_value(batch_output_file))
      (NUM_THREADS,
       po::value<int>(&num_threads)->default_value(num_threads));
  // clang-format on
  return desc;
}

void launcher_settings::print(std::ostream& out) const {
  out << "  " << MODE << ": " << mode << "\n"
      << "  " << MODULES << ": " << modules << "\n"
      << "  " << BATCH_INPUT_FILE << ": " << batch_input_file << "\n"
      << "  " << BATCH_OUTPUT_FILE << ": " << batch_output_file << "\n"
      << "  " << NUM_THREADS << ": " << num_threads;
}

}  // namespace launcher
}  // namespace motis
