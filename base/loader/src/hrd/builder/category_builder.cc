#include "motis/loader/hrd/builder/category_builder.h"

#include "parser/util.h"

#include "motis/core/common/get_or_create.h"
#include "motis/loader/hrd/files.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;
using namespace flatbuffers64;

category_builder::category_builder(std::map<uint32_t, category> hrd_categories)
    : hrd_categories_(std::move(hrd_categories)) {}

Offset<Category> category_builder::get_or_create_category(
    cstr category_str, flatbuffers64::FlatBufferBuilder& fbb) {
  auto const category_key = raw_to_int<uint32_t>(category_str);
  auto it = hrd_categories_.find(category_key);
  verify(it != end(hrd_categories_), "missing category: %.*s",
         (int)category_str.length(), category_str.c_str());

  return get_or_create(fbs_categories_, category_key, [&]() {
    return CreateCategory(fbb, to_fbs_string(fbb, it->second.name_, ENCODING),
                          it->second.output_rule_);
  });
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
