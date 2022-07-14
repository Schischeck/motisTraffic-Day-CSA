#pragma once

#include <ctime>

#include <string>
#include <utility>

namespace motis {
namespace loader {

class loader_options {
public:
  loader_options(std::string default_dataset, std::string schedule_begin,
                 int num_days = 2, bool write_serialized = false,
                 bool apply_rules = false, bool adjust_footpaths = true,
                 bool unique_check = true);

  std::pair<std::time_t, std::time_t> interval() const;

  std::string dataset_;
  std::string schedule_begin_;
  int num_days_;
  bool write_serialized_;
  bool unique_check_;
  bool apply_rules_;
  bool adjust_footpaths_;
};

}  // namespace loader
}  // namespace motis
