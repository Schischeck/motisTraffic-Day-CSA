#include "motis/loader/builders/hrd/station_builder.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;
using namespace parser;

Offset<String> line_builder::get_or_create_line(
    std::vector<parser::cstr> const& lines, FlatBufferBuilder& fbb) {
  if (lines.empty()) {
    return 0;
  } else {
    return get_or_create(fbs_lines_, raw_to_int<uint64_t>(lines[0]),
                         [&]() { return to_fbs_string(fbb, lines[0]); });
  }
}

}  // hrd
}  // loader
}  // motis
