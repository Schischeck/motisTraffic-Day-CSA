#include "motis/loader/parsers/hrd/categories_parser.h"

#include <vector>

#include "parser/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;
namespace fs = boost::filesystem;

void categories_parser::parse(loaded_file const& categories,
                              loaded_file const& providers) {}

Offset<String> categories_parser::get_category_info(uint32_t category_key) {
  auto it = fbs_categories_.find(category_key);
  verify(it != fbs_categories_.end(), "category missing");
  auto& fbs_category = it->second;
  if (!fbs_category.second) {
    fbs_category.second = true;
  }
  return fbs_category.first;
}

categories_parser::fbs_categories_t categories_parser::get_export_data(
    FlatBufferBuilder const& b) {
  categories_parser::fbs_categories_t s;
  return s;
}

}  // hrd
}  // loader
}  // motis
