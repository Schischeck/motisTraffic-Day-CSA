#include <iostream>

#include "boost/filesystem.hpp"

#include "conf/options_parser.h"
#include "conf/simple_config.h"
#include "parser/file.h"

#include "motis/geo/geojson.h"

#include "motis/routes/prepare/fbs/use_32bit_flatbuffers.h"
#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/routes/prepare/bus_stop_positions.h"
#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/seq/seq_graph_builder.h"
#include "motis/routes/prepare/seq/seq_graph_dijkstra.h"
#include "motis/routes/prepare/station_sequences.h"

#include "motis/routes/fbs/RoutesAuxiliary_generated.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "version.h"

namespace fs = boost::filesystem;
using namespace parser;
using namespace motis;
using namespace motis::loader;
using namespace motis::routes;
using namespace motis::geo;

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

  auto const sequences = load_station_sequences(schedule);
  auto const bus_stops = find_bus_stop_positions(schedule, opt.osm_);

  auto const rel_matches = match_osm_relations(opt.osm_, sequences, bus_stops);

  // std::vector<match_seq> test_matches;
  // station_seq seq;

  // seq.station_ids_ = {"0", "1", "2", "3", "4", "5", "6"};
  // seq.coordinates_ = {{1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}, {4.0, 0.0},
  //                     {5.0, 0.0}, {6.0, 0.0}, {8.0, 0.0}};

  // match_seq m1, m2, m3;
  // m1.polyline_.emplace_back(1.0, 0.0);
  // m1.polyline_.emplace_back(2.0, 0.0);
  // m1.polyline_.emplace_back(3.0, 0.0);

  // m1.stations_.emplace_back(0, 0);
  // m1.stations_.emplace_back(1, 1);
  // m1.stations_.emplace_back(2, 2);
  // test_matches.push_back(m1);

  // m3.polyline_.emplace_back(6.0, 0.0);
  // m3.polyline_.emplace_back(7.0, 0.0);
  // m3.stations_.emplace_back(4, 0);
  // m3.stations_.emplace_back(5, 1);
  // test_matches.push_back(m3);

  // m2.polyline_.emplace_back(4.0, 1.0);
  // m2.polyline_.emplace_back(5.0, 1.0);

  // m2.stations_.emplace_back(2, 0);
  // m2.stations_.emplace_back(3, 1);
  // test_matches.push_back(m2);

  //  build_graph(seq, test_matches);
  //  connect_matches(sequences, matches);

  std::vector<polyline> output;
  for (auto const& match : rel_matches) {
    for (auto const& m : match) {
      output.push_back(m.polyline_);
    }
  }
  dump_polylines(output);

  auto seq_graph = build_seq_graph(sequences[1792], rel_matches[1792]);

  for (auto& node : seq_graph.nodes_) {
    std::cout << "\n"
              << "station " << node->station_ << " " << node->idx_ << " : ";
    for (auto const& edge : node->edges_) {
      std::cout << edge.from_->station_ << "|" << edge.to_->station_ << " ";
    }
  }

  seq_graph_dijkstra dijkstra(seq_graph, {2}, {7});
  dijkstra.run();
  std::vector<latlng> polyline;

  for (auto const& e : dijkstra.get_links(7)) {
    std::cout << "\n" << e->from_->station_ << "|" << e->to_->station_;
  }

  //  flatbuffers::FlatBufferBuilder fbb;
  //  fbb.Finish(
  //      CreateRoutesAuxiliary(fbb, write_stop_positions(fbb,
  //      stop_positions)));
  //  parser::file(opt.out_.c_str(), "w+")
  //      .write(fbb.GetBufferPointer(), fbb.GetSize());
}
