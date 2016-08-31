#include <iostream>

#include "boost/filesystem.hpp"

#include "conf/options_parser.h"
#include "conf/simple_config.h"
#include "parser/file.h"

#include "motis/routes/prepare/fbs/use_32bit_flatbuffers.h"
#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/routes/prepare/bus_stop_positions.h"
#include "motis/routes/prepare/rel/osm_relations.h"
#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/station_sequences.h"

#include "motis/routes/fbs/RoutesAuxiliary_generated.h"
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
                            std::string const& out = "routes-auxiliary.raw")
      : simple_config("Prepare Options", "") {
    string_param(schedule_, schedule, "schedule", "/path/to/rohdaten");
    string_param(osm_, osm, "osm", "/path/to/germany-latest.osm.pbf");
    string_param(out_, out, "out", "/path/to/routes-auxiliary.raw");
  }

  std::string schedule_;
  std::string osm_;
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

  auto station_seq = load_station_sequences(schedule);

  auto relations = parse_relations(opt.osm_);

  auto polylines = aggregate_polylines(relations.relations_);

  auto matches = match_sequences(polylines, station_seq);
  std::cout << "\n" << matches.size();
  std::vector<std::vector<latlng>> output;
  for (auto const& match : matches) {
    if (match.full_match) {
      output.push_back(match.polyline_);
    }
  }
  std::cout << "\n" << output.size();
  write_geojson(output);

  // do_something(opt.osm_);

  //  auto stop_positions = find_bus_stop_positions(schedule, opt.osm_);

  //  flatbuffers::FlatBufferBuilder fbb;
  //  fbb.Finish(
  //      CreateRoutesAuxiliary(fbb, write_stop_positions(fbb,
  //      stop_positions)));
  //  parser::file(opt.out_.c_str(), "w+")
  //      .write(fbb.GetBufferPointer(), fbb.GetSize());
}
