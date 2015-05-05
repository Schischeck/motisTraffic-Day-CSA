#include "execution/dataset_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define DATASET_ROOT "dataset"

namespace td {
namespace execution {

namespace po = boost::program_options;

dataset_settings::dataset_settings(std::string default_dataset)
    : dataset(std::move(default_dataset)) {
}

po::options_description dataset_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()
    (DATASET_ROOT,
        po::value<std::string>(&dataset)->default_value(dataset),
        "TD Dataset root");
  return desc;
}

void dataset_settings::print(std::ostream& out) const {
  out << "  " << DATASET_ROOT << ": " << dataset;
}

}  // namespace execution
}  // namespace td
