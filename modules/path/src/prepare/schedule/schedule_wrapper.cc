#include "motis/path/prepare/schedule/schedule_wrapper.h"

#include "boost/filesystem.hpp"

#include "motis/path/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fs = boost::filesystem;

namespace motis {
namespace path {

schedule_wrapper::schedule_wrapper(std::string const& schedule_path) {
  auto schedule_file = fs::path(schedule_path) / "schedule.raw";
  if (!fs::is_regular_file(schedule_file)) {
    std::cerr << "cannot open schedule.raw\n";
    return 1;
  }

  schedule_buffer_ = file(schedule_file.string().c_str(), "r").content();
}

std::map<std::string, std::vector<latlng>> find_bus_stop_positions(
    std::string const& osm_file) const {
  return find_bus_stop_positions(GetSchedule(schedule_buffer_.buf_), osm_file);
}

std::vector<station_seq> load_station_sequences() {
  return load_station_sequences(GetSchedule(schedule_buffer_.buf_));
}

}  // namespace path
}  // namespace motis
