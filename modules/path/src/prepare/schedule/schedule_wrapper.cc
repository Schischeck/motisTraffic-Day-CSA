#include "motis/path/prepare/schedule/schedule_wrapper.h"

#include <iostream>

#include "boost/filesystem.hpp"

#include "parser/file.h"

#include "motis/path/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fs = boost::filesystem;
using namespace motis::loader;

namespace motis {
namespace path {

schedule_wrapper::schedule_wrapper(std::string const& schedule_path) {
  auto sched_file = fs::path(schedule_path) / "schedule.raw";
  if (!fs::is_regular_file(sched_file)) {
    std::cerr << "cannot open schedule.raw\n";  // XXX exception
  }

  schedule_buffer_ = parser::file(sched_file.string().c_str(), "r").content();
}

std::map<std::string, std::vector<geo::latlng>>
schedule_wrapper::find_bus_stop_positions(std::string const& osm_file) const {
  return motis::path::find_bus_stop_positions(
      GetSchedule(schedule_buffer_.buf_), osm_file);
}

std::vector<station_seq> schedule_wrapper::load_station_sequences() const {
  return motis::path::load_station_sequences(
      GetSchedule(schedule_buffer_.buf_));
}

}  // namespace path
}  // namespace motis
