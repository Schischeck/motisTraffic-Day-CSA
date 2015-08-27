#include "motis/loader/util.h"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {

void write_schedule(FlatBufferBuilder& b, boost::filesystem::path const& path) {
  file f(path.string().c_str(), "rw");
  f.write(b.GetBufferPointer(), b.GetSize());
}

buffer load_file(fs::path const& p) {
  return file(p.string().c_str(), "ro").content();
}

}  // namespace loader
}  // namespace motis
