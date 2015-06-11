#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace webservice {

class dataset_settings : public conf::configuration {
public:
  dataset_settings(std::string default_dataset);

  virtual ~dataset_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  std::string dataset;
};

}  // namespace webservice
}  // namespace motis
