#include "motis/webservice/dataset_settings.h"

#include <ostream>

#define DATASET "dataset"

namespace motis {
namespace webservice {

namespace po = boost::program_options;

dataset_settings::dataset_settings(std::string default_dataset)
    : dataset(std::move(default_dataset)) {}

po::options_description dataset_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()(DATASET,
                     po::value<std::string>(&dataset)->default_value(dataset),
                     "TD Dataset root");
  return desc;
}

void dataset_settings::print(std::ostream& out) const {
  out << "  " << DATASET << ": " << dataset;
}

}  // namespace webservice
}  // namespace motis
