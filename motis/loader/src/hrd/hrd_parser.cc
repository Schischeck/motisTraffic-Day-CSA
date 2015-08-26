#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <vector>
#include <string>

#include "boost/filesystem.hpp"

#include "parser/file.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

bool hrd_parser::applicable(fs::path const& path) {
  if (!fs::is_directory(path / "fahrten")) {
    return false;
  }

  std::vector<std::string> file_names = {ATTRIBUTES_FILE, STATIONS_FILE,
                                         COORDINATES_FILE, BITFIELDS_FILE,
                                         PLATFORMS_FILE};
  for (auto const& file_name : file_names) {
    if (!fs::is_regular_file(path / "stamm" / file_name)) {
      return false;
    }
  }

  return true;
}

buffer load_file(fs::path const& p) {
  return file(p.string().c_str(), "ro").content();
}

void hrd_parser::parse(fs::path const& path, FlatBufferBuilder& b) {
  auto stamm_path = path / "stamm";

  auto buf = load_file(stamm_path / ATTRIBUTES_FILE);
  auto attributes =
      parse_attributes({ATTRIBUTES_FILE, {buf.data(), buf.size()}});
}

}  // hrd
}  // loader
}  // motis
