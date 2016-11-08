#include <iostream>
#include <map>

#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "boost/filesystem.hpp"

#include "conf/options_parser.h"
#include "conf/simple_config.h"

#include "parser/file.h"

#include "motis/core/common/logging.h"

#include "motis/routes/db/rocksdb.h"
#include "motis/routes/prepare/bus_stop_positions.h"
#include "motis/routes/prepare/prepare_data.h"
#include "motis/routes/prepare/station_sequences.h"
#include "motis/routes/prepare/vector_utils.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "version.h"

namespace fs = boost::filesystem;
using namespace parser;
using namespace motis;
using namespace motis::loader;
using namespace motis::routes;

struct prepare_settings : public conf::simple_config {
  explicit prepare_settings(std::string const& schedule = "rohdaten",
                            std::string const& osm = "germany-latest.osm.pbf",
                            std::string const& extent = "germany-latest.poly",
                            std::string const& out = "routesdb")
      : simple_config("Prepare Options", "") {
    string_param(schedule_, schedule, "schedule", "/path/to/rohdaten");
    string_param(osm_, osm, "osm", "/path/to/germany-latest.osm.pbf");
    string_param(extent_, extent, "extent", "/path/to/germany-latest.poly");
    string_param(out_, out, "out", "/path/to/db");
  }

  std::string schedule_;
  std::string osm_;
  std::string extent_;
  std::string out_;
};

int main(int argc, char** argv) {
  prepare_settings opt;

  try {
    conf::options_parser parser({&opt});
    parser.read_command_line_args(argc, argv, false);

    if (parser.help()) {
      std::cout << "\n\troutes-prepare (MOTIS v" << short_version() << ")\n\n";
      parser.print_help(std::cout);
      return 0;
    } else if (parser.version()) {
      std::cout << "routes-prepare (MOTIS v" << long_version() << ")\n";
      return 0;
    }

    parser.read_configuration_file(false);
    parser.print_used(std::cout);
  } catch (std::exception const& e) {
    std::cout << "options error: " << e.what() << "\n";
    return 1;
  }

  auto schedule_file = fs::path(opt.schedule_) / "schedule.raw";
  if (!fs::is_regular_file(schedule_file)) {
    std::cerr << "cannot open schedule.raw\n";
    return 1;
  }

  auto const schedule_buf = file(schedule_file.string().c_str(), "r").content();
  auto const schedule = GetSchedule(schedule_buf.buf_);
  auto const stop_positions = find_bus_stop_positions(schedule, opt.osm_);

  auto sequences = load_station_sequences(schedule);

  // auto const extent_polygon = read_poly_file(opt.extent_);
  erase_if(sequences, [&](auto const& seq) {
    if (seq.categories_.empty() ||
        std::none_of(begin(seq.categories_), end(seq.categories_),
                     [](auto const& cat) { return cat < 6; })) {
      return true;
    }

    // if (std::any_of(begin(seq.coordinates_),
    // end(seq.coordinates_),
    //                 [&](auto const& coord) {
    //                   return !within(coord,
    //                   extent_polygon);
    //                 })) {
    //   return true;
    // }

    // if (seq.station_ids_.front() != "8000105" ||
    //     seq.station_ids_.back() != "8000126") {
    //   return true;
    // }

    return false;
  });

  rocksdb_database db(opt.out_);
  prepare(sequences, stop_positions, opt.osm_, db);
}
