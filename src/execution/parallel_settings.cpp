#include "execution/parallel_settings.h"

#include <ostream>
#include <thread>

#include "boost/program_options.hpp"

#define NUM_THREADS "num_threads"

namespace po = boost::program_options;

namespace td {
namespace execution {

parallel_settings::parallel_settings()
    : num_threads(std::thread::hardware_concurrency()) {
}

boost::program_options::options_description parallel_settings::desc() {
  po::options_description desc("Parallel Options");
  desc.add_options()
    (NUM_THREADS,
        po::value<int>(&num_threads)->default_value(num_threads),
        "number of concurrent execution threads");
  return desc;
}

void parallel_settings::print(std::ostream& out) const {
  out << "  " << NUM_THREADS << ": " << num_threads;
}

}  // namespace execution
}  // namespace td
