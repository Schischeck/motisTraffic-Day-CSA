#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class launcher_settings : public conf::configuration {
public:
  enum class motis_mode_t { BATCH, SERVER, TEST };

  launcher_settings(motis_mode_t m, std::string batch_input_file,
                    std::string batch_output_file, int num_threads);

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  motis_mode_t mode_;
  std::string batch_input_file_, batch_output_file_;
  int num_threads_;
};

}  // namespace launcher
}  // namespace motis
