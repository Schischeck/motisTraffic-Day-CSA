#pragma once

#include <ctime>
#include <string>
#include <utility>

namespace motis {
namespace loader {

class loader_options {
public:
  loader_options(std::string default_dataset, bool write_serialized, bool apply_rules,
                 bool adjust_footpaths, bool unique_check,
                 std::string schedule_begin, int num_days);

  std::pair<std::time_t, std::time_t> interval() const;

  std::string dataset_;
  bool write_serialized_;
  bool unique_check_;
  bool apply_rules_;
  bool adjust_footpaths_;
  std::string schedule_begin_;
  int num_days_;
};

}  // namespace loader
}  // namespace motis
