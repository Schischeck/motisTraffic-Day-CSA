#include "motis/loader/hrd/builder/line_builder.h"

#include "utl/get_or_create.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers64;
using namespace parser;

Offset<String> line_builder::get_or_create_line(
    std::vector<parser::cstr> const& lines, FlatBufferBuilder& fbb) {
  if (lines.empty()) {
    return 0;
  } else {
    return utl::get_or_create(fbs_lines_, raw_to_int<uint64_t>(lines[0]),
                              [&]() { return to_fbs_string(fbb, lines[0]); });
  }
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
