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
  category(std::string name, CategoryOutputRule output_rule)
      : name(std::move(name)), output_rule(output_rule) {}

  std::string name;
  CategoryOutputRule output_rule;
};

std::map<uint32_t, category> parse_categories(
    loaded_file const& categories_file);

}  // hrd
}  // loader
}  // motis
