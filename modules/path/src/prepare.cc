#include <iostream>
#include <map>
#include <memory>

// #include <valgrind/callgrind.h>

#include "boost/filesystem.hpp"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "conf/options_parser.h"
#include "conf/simple_config.h"

#include "geo/polygon.h"

#include "parser/file.h"

#include "utl/erase_if.h"

#include "motis/core/common/logging.h"

#include "motis/path/db/rocksdb.h"
#include "motis/path/prepare/db_statistics.h"
#include "motis/path/prepare/path_routing.h"
#include "motis/path/prepare/resolve_sequences.h"
#include "motis/path/prepare/schedule/bus_stop_positions.h"
#include "motis/path/prepare/schedule/schedule_wrapper.h"
#include "motis/path/prepare/schedule/stations.h"

#include "version.h"

namespace fs = boost::filesystem;
using namespace parser;
using namespace motis;
using namespace motis::loader;
using namespace motis::path;

struct prepare_settings : public conf::simple_config {
  explicit prepare_settings(std::string const& schedule = "rohdaten",
                            std::string const& osm = "germany-latest.osm.pbf",
                            std::string const& osrm = "osrm",
                            std::string const& out = "pathdb",
                            std::vector<std::string> const& filter = {},
                            bool stats_only = false)
      : simple_config("Prepare Options", "") {
    string_param(schedule_, schedule, "schedule", "/path/to/rohdaten");
    string_param(osm_, osm, "osm", "/path/to/germany-latest.osm.pbf");
    string_param(osrm_, osrm, "osrm", "path/to/osrm/files");
    string_param(out_, out, "out", "/path/to/db");
    multitoken_param(filter_, filter, "filter", "filter station sequences");
    bool_param(stats_only_, stats_only, "stats_only", "the state of 'out'");
  }

  std::string schedule_;
  std::string osm_;
  std::string osrm_;
  std::string out_;

  std::vector<std::string> filter_;

  bool stats_only_;
};

void filter_sequences(std::vector<std::string> const& filters,
                      std::vector<station_seq>& sequences) {
  motis::logging::scoped_timer timer("filter station sequences");
  for (auto const& filter : filters) {
    std::vector<std::string> tokens;
    boost::split(tokens, filter, boost::is_any_of(":"));
    verify(tokens.size() == 2, "unexpected filter");

    if (tokens[0] == "id") {
      utl::erase_if(sequences, [&tokens](auto const& seq) {
        return std::none_of(
            begin(seq.station_ids_), end(seq.station_ids_),
            [&tokens](auto const& id) { return id == tokens[1]; });
      });
    } else if (tokens[0] == "seq") {
      std::vector<std::string> ids;
      boost::split(ids, tokens[1], boost::is_any_of("."));
      utl::erase_if(sequences, [&ids](auto const& seq) {
        return ids == seq.station_ids_;
      });
    } else if (tokens[0] == "extent") {
      verify(fs::is_regular_file(tokens[1]), "cannot find extent polygon");
      auto const extent_polygon = geo::read_poly_file(tokens[1]);
      utl::erase_if(sequences, [&](auto const& seq) {
        return std::any_of(begin(seq.coordinates_), end(seq.coordinates_),
                           [&](auto const& coord) {
                             return !geo::within(coord, extent_polygon);
                           });
      });
    } else if (tokens[0] == "limit") {
      size_t const count = std::stoul(tokens[1]);
      sequences.resize(std::min(count, sequences.size()));
    } else {
      LOG(motis::logging::info) << "unknown filter: " << tokens[0];
    }
  }
}

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

  if (!opt.stats_only_) {
    verify(fs::is_regular_file(opt.osrm_), "cannot find osrm dataset");
    verify(fs::is_regular_file(opt.osm_), "cannot find osm dataset");

    // std::map<std::string, std::vector<geo::latlng>> stop_positions;
    std::vector<station_seq> sequences;
    {
      schedule_wrapper sched{opt.schedule_};
      // stop_positions = sched.find_bus_stop_positions(opt.osm_);
      sequences = sched.load_station_sequences();
    }
    auto const stations = all_stations(sequences);

    filter_sequences(opt.filter_, sequences);
    LOG(motis::logging::info) << "station sequences: " << sequences.size();

    auto routing = make_path_routing(stations, opt.osm_, opt.osrm_);
    db_builder builder(std::make_unique<rocksdb_database>(opt.out_));

    // CALLGRIND_START_INSTRUMENTATION;
    resolve_sequences(sequences, routing, builder);
    // CALLGRIND_STOP_INSTRUMENTATION;
    // CALLGRIND_DUMP_STATS;

    builder.finish();
  }

  auto const db = std::make_unique<rocksdb_database>(opt.out_);
  dump_db_statistics(*db);
  std::cout << std::endl;
}
