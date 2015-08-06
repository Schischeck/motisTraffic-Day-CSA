#include "motis/loader/util.h"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"

using namespace flatbuffers;
using namespace parser;

namespace motis {
namespace loader {

void write_schedule(FlatBufferBuilder& b, std::string const& path) {
  file f(path.c_str(), "rw");
  f.write(b.GetBufferPointer(), b.GetSize());
}

}  // namespace loader
}  // namespace motis
