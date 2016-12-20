#include <iostream>
#include <map>
#include <memory>

#include "motis/path/prepare/fbs/use_64bit_flatbuffers.h"

#include "boost/filesystem.hpp"

#include "utl/erase_if.h"

#include "conf/options_parser.h"
#include "conf/simple_config.h"

#include "parser/file.h"

#include "motis/core/common/logging.h"

#include "motis/path/db/rocksdb.h"
#include "motis/path/prepare/bus_stop_positions.h"
#include "motis/path/prepare/prepare_data.h"
#include "motis/path/prepare/rel/polyline_aggregator.h"
#include "motis/path/prepare/rel/relation_parser.h"
#include "motis/path/prepare/routing/osrm_routing.h"
#include "motis/path/prepare/routing/relation_routing.h"
#include "motis/path/prepare/routing/stub_routing.h"
#include "motis/path/prepare/station_sequences.h"

#include "motis/schedule-format/Schedule_generated.h"

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

  auto schedule_file = fs::path(opt.schedule_) / "schedule.raw";
  if (!fs::is_regular_file(schedule_file)) {
    std::cerr << "cannot open schedule.raw\n";
    return 1;
  }

  fs::path path(opt.osrm_);
  auto directory = path.parent_path();
  if (!is_directory(directory)) {
    throw std::runtime_error("OSRM dataset is not a folder!");
  }

  auto const& profile = directory.filename().string();
  motis::logging::scoped_timer timer("loading OSRM dataset: " + profile);
  auto const schedule_buf = file(schedule_file.string().c_str(), "r").content();
  auto const schedule = GetSchedule(schedule_buf.buf_);
  auto stop_positions = find_bus_stop_positions(schedule, opt.osm_);

  auto sequences = load_station_sequences(schedule);
  motis::logging::manual_timer poly_timer("loading poly");
  auto const extent_polygon = geo::read_poly_file(opt.extent_);
  poly_timer.stop_and_print();

  utl::erase_if(sequences, [&](auto const& seq) {
    return std::any_of(
        begin(seq.coordinates_), end(seq.coordinates_),
        [&](auto const& coord) { return !geo::within(coord, extent_polygon); });
  });

  auto const relations = parse_relations(opt.osm_);
  LOG(motis::logging::info) << "found " << relations.relations_.size()
                            << " relations";
  auto const polylines = aggregate_polylines(relations.relations_);

  rocksdb_database db(opt.out_);
  strategies routing_strategies(
      std::make_unique<stub_routing>(0),
      std::make_unique<relation_routing>(1, polylines));
  strategy_prepare(sequences, routing_strategies, db);
}
