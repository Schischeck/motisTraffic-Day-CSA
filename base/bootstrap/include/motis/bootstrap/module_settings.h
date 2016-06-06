#pragma once

#include <string>
#include <vector>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace bootstrap {

class module_settings : public conf::configuration {
public:
  explicit module_settings(std::vector<std::string> modules);

  ~module_settings() override = default;

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  std::vector<std::string> modules_;
};

}  // namespace bootstrap
}  // namespace motis
