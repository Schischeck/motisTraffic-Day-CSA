#pragma once

#include <ctime>
#include <string>
#include <utility>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class dataset_settings : public conf::configuration {
public:
  dataset_settings(std::string default_dataset, bool ignore_serialized,
                   bool unique_check, std::string schedule_begin, int num_days);

  virtual ~dataset_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  std::pair<std::time_t, std::time_t> interval() const;

  std::string dataset;
  bool use_serialized;
  bool unique_check;
  std::string schedule_begin;
  int num_days;
};

}  // namespace launcher
}  // namespace motis
