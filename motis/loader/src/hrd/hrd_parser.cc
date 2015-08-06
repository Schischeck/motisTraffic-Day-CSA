#include "motis/loader/parsers/hrd/hrd_parser.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"

using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

bool hrd_parser::applicable(std::string const& path) { return true; }

/*
 * TODO(tobias) remove / implement
std::vector<Offset<Train>> read_trains(FlatBufferBuilder& b,
                                       std::string const& path) {
  std::vector<Offset<Train>> trains;
  trains.push_back(CreateTrain(b, )) return trains;
}
*/

void hrd_parser::parse(std::string const& path) {
  FlatBufferBuilder b;

  // TODO(tobias) remove / implement
  // CreateSchedule(b, b.CreateVector(reat_trains(b, path)));

  write_schedule(b, path);
}

}  // hrd
}  // loader
}  // motis
