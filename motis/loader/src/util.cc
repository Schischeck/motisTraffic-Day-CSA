#include "motis/loader/util.h"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {

int hhmm_to_min(int hhmm) {
  if (hhmm < 0) {
    return hhmm;
  } else {
    return (hhmm / 100) * 60 + (hhmm % 100);
  }
}

void write_schedule(FlatBufferBuilder& b, boost::filesystem::path const& path) {
  file f(path.string().c_str(), "w+");
  f.write(b.GetBufferPointer(), b.GetSize());
}

buffer load_file(fs::path const& p) {
  return file(p.string().c_str(), "r").content();
}

}  // namespace loader
}  // namespace motis
