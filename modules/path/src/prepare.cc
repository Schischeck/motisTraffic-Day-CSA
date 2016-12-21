#include <iostream>
#include <map>
#include <memory>

#include "boost/filesystem.hpp"

#include "conf/options_parser.h"
#include "conf/simple_config.h"

#include "geo/polygon.h"

#include "parser/file.h"

#include "utl/erase_if.h"

#include "motis/core/common/logging.h"

#include "motis/path/db/rocksdb.h"
#include "motis/path/prepare/bus_stop_positions.h"
#include "motis/path/prepare/rel/polyline_aggregator.h"
#include "motis/path/prepare/rel/relation_parser.h"
#include "motis/path/prepare/resolve_sequences.h"
#include "motis/path/prepare/routing/path_routing.h"
#include "motis/path/prepare/schedule/schedule_wrapper.h"

#include "version.h"

namespace fs = boost::filesystem;
using namespace parser;
using namespace motis;
using namespace motis::loader;
using namespace motis::path;

struct prepare_settings : public conf::simple_config {
  explicit prepare_settings(std::string const& schedule = "rohdaten",
                            std::string const& osm = "germany-latest.osm.pbf",
                            std::string const& extent = "germany-latest.poly",
                            std::string const& out = "routesdb",
                            std::string const& osrm = "osrm")
      : simple_config("Prepare Options", "") {
    string_param(schedule_, schedule, "schedule", "/path/to/rohdaten");
    string_param(osm_, osm, "osm", "/path/to/germany-latest.osm.pbf");
    string_param(extent_, extent, "extent", "/path/to/germany-latest.poly");
    string_param(out_, out, "out", "/path/to/db");
    string_param(osrm_, osrm, "osrm", "path/to/osrm/files");
  }

  std::string schedule_;
  std::string osm_;
  std::string extent_;
  std::string out_;
  std::string osrm_;
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

  fs::path path(opt.osrm_);
  auto directory = path.parent_path();
  if (!is_directory(directory)) {
    throw std::runtime_error("OSRM dataset is not a folder!");
  }

  std::map<std::string, std::vector<geo::latlng>> stop_positions;
  std::vector<station_seq> sequences;
  {
    schedule_wrapper sched{opt.schedule_};
    stop_positions = sched.find_bus_stop_positions(opt.osm_);
    sequences = sched.load_station_sequences();
  }

  auto const extent_polygon = geo::read_poly_file(opt.extent_);
  utl::erase_if(sequences, [&](auto const& seq) {
    return std::any_of(
        begin(seq.coordinates_), end(seq.coordinates_),
        [&](auto const& coord) { return !geo::within(coord, extent_polygon); });
  });

  auto const relations = parse_relations(opt.osm_);
  LOG(motis::logging::info) << "found " << relations.relations_.size()
                            << " relations";
  auto const polylines = aggregate_polylines(relations.relations_);

  auto&& routing = make_path_routing();
  db_builder builder(std::make_unique<rocksdb_database>(opt.out_));

  // XXX debugging
  std::vector<station_seq> debug_seq;
  for (auto const seq : sequences) {
    if (std::find(begin(seq.station_ids_), end(seq.station_ids_), "0104736") !=
        end(seq.station_ids_)) {
      debug_seq.push_back(seq);
      break;
    }
  }

  resolve_sequences(debug_seq, routing, builder);
}
