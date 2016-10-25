#include <iostream>

#include <map>

#include "boost/filesystem.hpp"

#include "conf/options_parser.h"
#include "conf/simple_config.h"
#include "geo/polygon.h"
#include "parser/file.h"

#include "motis/core/common/transform_to_vec.h"

#include "motis/routes/prepare/fbs/use_32bit_flatbuffers.h"
#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/routes/prepare/bus_stop_positions.h"
#include "motis/routes/prepare/geojson.h"
#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/seq/seq_graph_builder.h"
#include "motis/routes/prepare/seq/seq_graph_dijkstra.h"
#include "motis/routes/prepare/station_sequences.h"
#include "motis/routes/prepare/vector_utils.h"

#include "motis/routes/fbs/RoutesAuxiliary_generated.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "version.h"

namespace fs = boost::filesystem;
using namespace parser;
using namespace motis;
using namespace motis::loader;
using namespace motis::routes;
using namespace geo;

struct prepare_settings : public conf::simple_config {
  explicit prepare_settings(std::string const& schedule = "rohdaten",
                            std::string const& osm = "germany-latest.osm.pbf",
                            std::string const& extent = "germany-latest.poly",
                            std::string const& out = "routes-auxiliary.raw")
      : simple_config("Prepare Options", "") {
    string_param(schedule_, schedule, "schedule", "/path/to/rohdaten");
    string_param(osm_, osm, "osm", "/path/to/germany-latest.osm.pbf");
    string_param(extent_, extent, "extent", "/path/to/germany-latest.poly");
    string_param(out_, out, "out", "/path/to/routes-auxiliary.raw");
  }

  std::string schedule_;
  std::string osm_;
  std::string extent_;
  std::string out_;
};

struct stats {

  std::string report() {
    std::string out = "";
    out += "\nSequences: " + std::to_string(seqs_);
    out += "\nNot broken: " + std::to_string(not_broken_) + " / " +
           percentage(not_broken_, seqs_) + "%";
    out += "\nUsing relations: " + std::to_string(matched_) + " / " +
           percentage(matched_, seqs_) + "%";
    out += "\nEdges: " + std::to_string(edges_);
    out += "\nAirlines: " + std::to_string(airlines_) + " / " +
           percentage(airlines_, edges_) + "%";
    out += "\nIn station: " + std::to_string(edges_in_station_) + " / " +
           percentage(edges_in_station_, edges_) + "%";
    out += "\nAirlines without stations: " + std::to_string(airlines_without_) +
           " / " + percentage(airlines_without_, edges_) + "%";
    return out;
  }

  std::string percentage(int share, int total) {
    if (total < 0) return std::to_string(0);
    return std::to_string(((float)share / (float)total) * 100);
  }

  int airlines_ = 0;
  int airlines_without_ = 0;
  int edges_in_station_ = 0;
  int not_broken_ = 0;
  int edges_ = 0;
  int seqs_ = 0;
  int matched_ = 0;
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

  auto const extent_polygon = read_poly_file(opt.extent_);
  sequences.erase(
      std::remove_if(
          begin(sequences), end(sequences),
          [&](auto const& seq) {
            if (seq.categories_.empty() ||
                std::none_of(begin(seq.categories_), end(seq.categories_),
                             [](auto const& cat) { return cat < 6; })) {
              return true;
            }

            if (std::any_of(begin(seq.coordinates_), end(seq.coordinates_),
                            [&](auto const& coord) {
                              return !within(coord, extent_polygon);
                            })) {
              return true;
            }

            // if (seq.station_ids_.front() != "8000105" ||
            //     seq.station_ids_.back() != "8000126") {
            //   return true;
            // }

            return false;
          }),
      end(sequences));

  auto const rel_matches =
      match_osm_relations(opt.osm_, sequences, stop_positions);

  flatbuffers::FlatBufferBuilder fbb;
  std::vector<flatbuffers::Offset<motis::routes::Route>> fbs_routes;
  stats stats;
  stats.seqs_ = sequences.size();

  for (auto i = 0u; i < sequences.size(); ++i) {
    auto const& seq = sequences[i];
    auto const& relations = rel_matches[i];
    std::cout << "\nusing " << relations.size() << "relations";
    if (relations.size() > 0) {
      stats.matched_++;
    }
    auto fbs_stations = transform_to_vec(
        seq.station_ids_,
        [&](auto const& station_id) { return fbb.CreateString(station_id); });

    for (auto const& category_group : category_groups(seq.categories_)) {
      stub_routing strategy{seq};
      auto const g =
          build_seq_graph(category_group.first, seq, relations, strategy);

      seq_graph_dijkstra dijkstra(g, g.initials_, g.goals_);
      dijkstra.run();

      // for (auto const& goal : g.goals_) {
      //   std::cout << '\n' << dijkstra.get_distance(goal);
      // }

      auto best_goal_it =
          std::min_element(begin(g.goals_), end(g.goals_),
                           [&](auto const& lhs_idx_, auto const& rhs_idx_) {
                             return dijkstra.get_distance(lhs_idx_) <
                                    dijkstra.get_distance(rhs_idx_);
                           });
      if (best_goal_it == end(g.goals_)) {
        std::cout << "\n no result";
        continue;
      }

      std::cout << '\n' << dijkstra.get_distance(*best_goal_it);

      std::vector<std::vector<geo::latlng>> lines{seq.station_ids_.size() - 1};
      float save = stats.airlines_;
      for (auto const& edge : dijkstra.get_links(*best_goal_it)) {
        // std::cout << "\n"
        //           << edge->from_->idx_ << " -> " << edge->to_->idx_ << "("
        //           << edge->weight_ << ")";
        stats.edges_++;
        if (edge->source_.type_ == source_spec::type::AIRLINE) {
          stats.airlines_++;
        }
        if (edge->source_.station_) {
          stats.edges_in_station_++;
          if (edge->source_.type_ == source_spec::type::AIRLINE) {
            stats.airlines_without_++;
          }
        }
        append(lines[edge->from_->station_idx_], edge->p_);
      }
      if (save == stats.airlines_) {
        stats.not_broken_++;
      }
      // std::stringstream ss;
      // ss << "debug/polyline." << seq.station_ids_.front() << "-"
      //    << seq.station_ids_.back() << "." << i << ".json";
      // dump_polylines(lines, ss.str().c_str());
      std::vector<flatbuffers::Offset<Polyline>> fbs_lines;
      for (auto const& line : lines) {
        std::vector<double> flat_polyline;
        for (auto const& latlng : line) {
          flat_polyline.push_back(latlng.lat_);
          flat_polyline.push_back(latlng.lng_);
        }
        fbs_lines.push_back(
            CreatePolyline(fbb, fbb.CreateVector(flat_polyline)));
      }

      fbs_routes.push_back(CreateRoute(fbb, fbb.CreateVector(fbs_stations),
                                       fbb.CreateVector(category_group.second),
                                       fbb.CreateVector(fbs_lines)));
    }
  }
  std::cout << "\n" << stats.report();
  std::cout << "\n";
  fbb.Finish(CreateRoutesAuxiliary(fbb,
                                   write_stop_positions(fbb, stop_positions),
                                   fbb.CreateVector(fbs_routes)));
  parser::file(opt.out_.c_str(), "w+")
      .write(fbb.GetBufferPointer(), fbb.GetSize());
}
