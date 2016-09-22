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

  for (auto i = 0u; i < rel_matches.size(); i++) {
    if (rel_matches[i].size() > 1) {
      std::cout << i << "|";
    }
  }

  //  auto const& g = build_seq_graph(sequences[26344], rel_matches[26344]);
  //  auto const& g = build_seq_graph(sequences[26346], rel_matches[26346]);
  auto const& g = build_seq_graph(sequences[26395], rel_matches[26395]);

  std::cout << "\n Graph Nodes:" << g.nodes_.size();
  std::cout << "\n Station Seq:" << sequences[26395].station_ids_.size();
  std::cout << "\n Station to Node: " << g.station_to_nodes_.size();

  auto index = 0;
  for (auto const& s : g.station_to_nodes_) {
    std::cout << "\nStation " << index << "|"
              << sequences[26395].coordinates_[index].lat_ << ","
              << sequences[26395].coordinates_[index].lng_ << ":";
    for (auto const& n : s) {
      std::cout << n->idx_;
      for (auto const& edge : n->edges_) {
        std::cout << "(" << edge.from_->idx_ << "|" << edge.to_->idx_
                  << "|w:" << edge.weight_ << ") ";
      }
    }
    index++;
  }

  std::cout << "\n Goals:";

  for (auto const& n : g.goals_) {
    std::cout << n << " ";
  }

  std::cout << "\n Inits:";

  for (auto const& n : g.initials_) {
    std::cout << n << " ";
  }

  seq_graph_dijkstra dijkstra(g, g.initials_, g.goals_);
  std::vector<std::vector<geo::latlng>> polylines;
  dijkstra.run();
  for (auto const& goal : g.goals_) {
    std::vector<geo::latlng> line;
    for (auto const& edge : dijkstra.get_links(goal)) {
      std::cout << "\n"
                << edge->from_->idx_ << "|" << edge->to_->idx_
                << "|w:" << edge->weight_;
      for (auto const& p : edge->p_) {
        line.push_back(p);
      }
    }
    polylines.push_back(line);
  }

  dump_polylines(polylines);

  //  flatbuffers::FlatBufferBuilder fbb;
  //  fbb.Finish(
  //      CreateRoutesAuxiliary(fbb, write_stop_positions(fbb,
  //      stop_positions)));
  //  parser::file(opt.out_.c_str(), "w+")
  //      .write(fbb.GetBufferPointer(), fbb.GetSize());
}
