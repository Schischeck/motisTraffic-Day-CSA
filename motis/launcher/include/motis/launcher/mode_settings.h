#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class mode_settings : public conf::configuration {
public:
  enum motis_mode_t { BATCH, SERVER, TEST };

  mode_settings(motis_mode_t mode);

  virtual ~mode_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  motis_mode_t mode;
};

}  // namespace launcher
}  // namespace motis
