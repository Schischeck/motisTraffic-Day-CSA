#pragma once

#include <cinttypes>
#include <string>

namespace motis {

struct category {
  category(std::string name, uint8_t output_rule)
      : name(name), output_rule(output_rule){};

  std::string name;
  uint8_t output_rule;
};

}  // namespace motis
