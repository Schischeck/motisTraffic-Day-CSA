#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace webservice {

class modules_settings : public conf::configuration {
public:
  modules_settings(std::string default_modules_path);

  virtual ~modules_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  std::string modules_path;
};

}  // namespace webservice
}  // namespace motis
