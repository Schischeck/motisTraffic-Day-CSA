#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class launcher_settings : public conf::configuration {
public:
  enum class motis_mode_t { BATCH, SERVER, TEST };

  launcher_settings(motis_mode_t mode, std::vector<std::string> modules,
                    std::string batch_input_file, std::string batch_output_file,
                    int num_threads);

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  motis_mode_t mode;
  std::vector<std::string> modules;
  std::string batch_input_file, batch_output_file;
  int num_threads;
};

}  // namespace launcher
}  // namespace motis
