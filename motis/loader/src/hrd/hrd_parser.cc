#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <vector>
#include <string>

#include "boost/filesystem.hpp"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace flatbuffers;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

bool hrd_parser::applicable(fs::path const& path) {
  if (!fs::is_directory(path / "fahrten")) {
    return false;
  }

  std::vector<std::string> file_names = {HRD_ATTRIBUTES};

  for (auto const& file_name : file_names) {
    if (!fs::is_regular_file(path / "stamm" / file_name)) {
      return false;
    }
  }

  return true;
}

/*
 * TODO(tobias) remove / implement
std::vector<Offset<Train>> read_trains(FlatBufferBuilder& b,
                                       boost::filesystem::path const& path) {
  std::vector<Offset<Train>> trains;
  trains.push_back(CreateTrain(b, )) return trains;
}
*/

void hrd_parser::parse(fs::path const& path) {
  FlatBufferBuilder b;

  // TODO(tobias) remove / implement
  // CreateSchedule(b, b.CreateVector(reat_trains(b, path)));

  write_schedule(b, path);
}

}  // hrd
}  // loader
}  // motis
