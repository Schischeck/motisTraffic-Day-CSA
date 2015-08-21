#include "motis/loader/util.h"

#include "boost/locale.hpp"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"

using namespace flatbuffers;
using namespace parser;

namespace motis {
namespace loader {

flatbuffers::Offset<flatbuffers::String> to_fbs_string(
    flatbuffers::FlatBufferBuilder& b, std::string const& s,
    std::string const& charset) {
  auto encoded = boost::locale::conv::to_utf<char>(s, charset);
  return b.CreateString(encoded);
}

flatbuffers::Offset<flatbuffers::String> to_fbs_string(
    flatbuffers::FlatBufferBuilder& b, parser::cstr const& s,
    std::string const& charset) {
  std::string cppstr(s.str, s.len);
  return to_fbs_string(b, cppstr, charset);
}

void write_schedule(FlatBufferBuilder& b, boost::filesystem::path const& path) {
  file f(path.c_str(), "rw");
  f.write(b.GetBufferPointer(), b.GetSize());
}

}  // namespace loader
}  // namespace motis
