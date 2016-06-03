#pragma once

#include <cinttypes>
#include <string>

namespace motis {

struct category {
  category(std::string name, uint8_t output_rule)
      : name_(std::move(name)), output_rule_(output_rule) {}

  std::string name_;
  uint8_t output_rule_;
};

}  // namespace motis
