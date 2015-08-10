#include "motis/loader/parsers/hrd/attributes_parser.h"

#include "parser/file.h"
#include "parser/cstr.h"

#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

std::vector<Offset<Attribute>> parse_attributes(FlatBufferBuilder& b,
                                                cstr file_content) {
  std::vector<Offset<Attribute>> attributes;
  for_each_line(file_content, [&](cstr line) {
    if (line.len < 13) {
      return;
    }
    auto code = to_fbs_string(b, line.substr(0, size(2)));
    auto text = to_fbs_string(b, line.substr(12, line.len - 1));
    attributes.emplace_back(CreateAttribute(b, code, text));
  });
  return attributes;
}

}  // hrd
}  // loader
}  // motis
