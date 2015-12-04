#include "motis/loader/util.h"

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

void write_schedule(FlatBufferBuilder& b, fs::path const& path) {
  file f(path.string().c_str(), "w+");
  f.write(b.GetBufferPointer(), b.GetSize());
}

buffer load_file(fs::path const& p) {
  return file(p.string().c_str(), "r").content();
}

void collect_files(fs::path const& root, std::vector<fs::path>& files) {
  for (auto const& entry : fs::recursive_directory_iterator(root)) {
    if (fs::is_regular(entry.path())) {
      files.push_back(entry.path());
    }
  }
}

}  // namespace loader
}  // namespace motis
