#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class modules_settings : public conf::configuration {
public:
  modules_settings(std::string default_path);

  virtual ~modules_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  std::string path;
};

}  // namespace launcher
}  // namespace motis
