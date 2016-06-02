#pragma once

#include <cinttypes>
#include <map>
#include <string>

#include "motis/schedule-format/Category_generated.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct category {
  category() = default;
  category(std::string name, uint8_t output_rule)
      : name_(std::move(name)), output_rule_(output_rule) {}

  std::string name_;
  uint8_t output_rule_;
};

std::map<uint32_t, category> parse_categories(loaded_file const& file);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
