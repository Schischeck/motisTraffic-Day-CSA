#pragma once

#include <cinttypes>
#include <map>

#include "parser/cstr.h"

#include "motis/loader/hrd/parser/categories_parser.h"

#include "motis/schedule-format/Category_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct category_builder {
  category_builder(std::map<uint32_t, category> hrd_categories);

  flatbuffers::Offset<Category> get_or_create_category(
      parser::cstr, flatbuffers::FlatBufferBuilder&);

  std::map<uint32_t, category> const hrd_categories_;
  std::map<uint32_t, flatbuffers::Offset<Category>> fbs_categories_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
