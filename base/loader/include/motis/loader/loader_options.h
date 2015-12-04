#pragma once

#include <ctime>
#include <utility>
#include <string>

namespace motis {
namespace loader {

class loader_options {
public:
  loader_options(std::string dataset, bool write_serialized, bool unique_check,
                 bool apply_rules, std::string schedule_begin, int num_days);

  std::pair<std::time_t, std::time_t> interval() const;

  std::string dataset;
  bool write_serialized;
  bool unique_check;
  bool apply_rules;
  std::string schedule_begin;
  int num_days;
};

}  // namespace loader
}  // namespace motis
