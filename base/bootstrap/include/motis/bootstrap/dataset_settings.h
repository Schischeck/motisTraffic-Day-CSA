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
                   bool unique_check, bool apply_rules,
                   std::string schedule_begin, int num_days);

  virtual ~dataset_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
};

}  // namespace bootstrap
}  // namespace motis
