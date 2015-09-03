#pragma once

#include <cinttypes>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/schedule-format/Category_generated.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct categories_parser {
  struct category {
    category(std::string name, CategoryOutputRule output_rule)
        : name(std::move(name)), output_rule(output_rule) {}

    std::string name;
    CategoryOutputRule output_rule;
  };

  void parse(loaded_file const& categories_file);

  std::map<uint32_t, category> fbs_categories_;
};

}  // hrd
}  // loader
}  // motis
