#include "motis/loader/parsers/hrd/attributes_parser.h"

#include "parser/file.h"
#include "parser/cstr.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

std::vector<Offset<Attribute>> parse_attributes(
    flatbuffers::FlatBufferBuilder& b, boost::filesystem::path const& path) {
  auto buf = file(path, "ro").content();

  std::vector<Offset<Attribute>> attributes;
  for_each_line({buf.buf_, buf.size_}, [&attributes](cstr line) {
    auto code = b.CreateString(line.substr(0, size(2)));
    auto text = b.CreateString(line.substr(12, line.len - 1));
    attributes.emplace_back()
  });
  return attributes;
}

}  // hrd
}  // loader
}  // motis
