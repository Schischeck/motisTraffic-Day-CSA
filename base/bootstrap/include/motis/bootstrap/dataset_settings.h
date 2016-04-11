#pragma once

#include <ctime>
#include <string>
#include <utility>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

#include "motis/loader/loader_options.h"

namespace motis {
namespace bootstrap {

class dataset_settings : public conf::configuration,
                         public motis::loader::loader_options {
public:
  dataset_settings(std::string default_dataset, bool write_serialized,
                   bool apply_rules, bool adjust_footpaths, bool unique_check,
                   std::string schedule_begin, int num_days);

  ~dataset_settings() override = default;

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
};

}  // namespace bootstrap
}  // namespace motis
