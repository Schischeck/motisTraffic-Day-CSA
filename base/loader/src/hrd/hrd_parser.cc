#include "motis/loader/hrd/hrd_parser.h"

#include "motis/core/common/logging.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/hrd/builder/attribute_builder.h"
#include "motis/loader/hrd/builder/bitfield_builder.h"
#include "motis/loader/hrd/builder/category_builder.h"
#include "motis/loader/hrd/builder/direction_builder.h"
#include "motis/loader/hrd/builder/footpath_builder.h"
#include "motis/loader/hrd/builder/line_builder.h"
#include "motis/loader/hrd/builder/meta_station_builder.h"
#include "motis/loader/hrd/builder/provider_builder.h"
#include "motis/loader/hrd/builder/route_builder.h"
#include "motis/loader/hrd/builder/rule_service_builder.h"
#include "motis/loader/hrd/builder/service_builder.h"
#include "motis/loader/hrd/builder/station_builder.h"
#include "motis/loader/hrd/files.h"
#include "motis/loader/hrd/parser/attributes_parser.h"
#include "motis/loader/hrd/parser/basic_info_parser.h"
#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/hrd/parser/categories_parser.h"
#include "motis/loader/hrd/parser/directions_parser.h"
#include "motis/loader/hrd/parser/merge_split_rules_parser.h"
#include "motis/loader/hrd/parser/providers_parser.h"
#include "motis/loader/hrd/parser/service_parser.h"
#include "motis/loader/hrd/parser/stations_parser.h"
#include "motis/loader/hrd/parser/through_services_parser.h"
#include "motis/loader/hrd/parser/timezones_parser.h"
#include "motis/loader/hrd/parser/track_rules_parser.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers64;
using namespace parser;
using namespace motis::logging;
namespace fs = boost::filesystem;

enum filename_key {
  ATTRIBUTES,
  STATIONS,
  COORDINATES,
  BITFIELDS,
  TRACKS,
  INFOTEXT,
  BASIC_DATA,
  CATEGORIES,
  DIRECTIONS,
  PROVIDERS,
  THROUGH_SERVICES,
  MERGE_SPLIT_SERVICES,
  TIMEZONES,
  FOOTPATHS,
  FOOTPATHS_EXT
};

bool hrd_parser::applicable(fs::path const& path) {
  std::vector<config> configs = {hrd_5_00_8_, hrd_5_20_26_};
  return std::any_of(begin(configs), end(configs),
                     [&path, this](config c) { return applicable(path, c); });
}

bool hrd_parser::applicable(fs::path const& path, config const& c) {
  auto const core_data_root = path / CORE_DATA;
  return fs::is_directory(path / SCHEDULE_DATA) &&
         std::all_of(
             begin(c.files_.required_files_), end(c.files_.required_files_),
             [&core_data_root](std::vector<std::string> const& alternatives) {
               if (alternatives.empty()) {
                 return true;
               }
               return std::any_of(
                   begin(alternatives), end(alternatives),
                   [&core_data_root](std::string const& filename) {
                     return fs::is_regular_file(core_data_root / filename);
                   });
             });
}

std::vector<std::string> hrd_parser::missing_files(
    fs::path const& hrd_root) const {
  auto const core_data_root = hrd_root / CORE_DATA;
  if (fs::is_regular_file(core_data_root / "eckdaten.101")) {
    return missing_files(hrd_root, hrd_5_00_8_);
  }
  if (fs::is_regular_file(core_data_root / "eckdaten.txt")) {
    return missing_files(hrd_root, hrd_5_20_26_);
  }
  return {"eckdaten.*"};
}

std::vector<std::string> hrd_parser::missing_files(fs::path const& hrd_root,
                                                   config const& c) const {
  std::vector<std::string> missing_files;
  auto const schedule_data_root = hrd_root / SCHEDULE_DATA;
  if (!fs::is_directory(schedule_data_root)) {
    missing_files.push_back(schedule_data_root.string());
  }
  auto const core_data_root = hrd_root / CORE_DATA;
  for (auto const& alternatives : c.files_.required_files_) {
    std::vector<int> missing_indices;
    int pos = 0;
    for (auto const& alternative : alternatives) {
      if (!fs::is_regular_file(
              (core_data_root / alternative).string().c_str())) {
        missing_indices.push_back(pos);
      }
      ++pos;
    }
    if (missing_indices.size() < alternatives.size()) {
      continue;
    }
    for (auto const idx : missing_indices) {
      missing_files.emplace_back(alternatives[idx]);
    }
  }
  return missing_files;
}

loaded_file load(fs::path const& root, filename_key k,
                 std::vector<std::vector<std::string>> const& required_files) {
  auto it = std::find_if(begin(required_files[k]), end(required_files[k]),
                         [&root](std::string const& filename) {
                           return fs::is_regular_file(root / filename);
                         });
  verify(it != end(required_files[k]),
         "unable to load non-regular file(s): filename_key=%d", (int)k);
  return loaded_file(root / *it);
}

void parse_and_build_services(
    fs::path const& hrd_root, std::map<int, bitfield> const& bitfields,
    std::vector<std::unique_ptr<loaded_file>>& schedule_data,
    std::function<void(hrd_service const&)> const& service_builder_fun,
    config const& c) {
  auto const schedule_data_root = hrd_root / SCHEDULE_DATA;

  std::vector<fs::path> files;
  collect_files(schedule_data_root, files);

  int count = 0;
  for (auto const& file : files) {
    schedule_data.emplace_back(std::make_unique<loaded_file>(file));
    LOG(info) << "parsing " << ++count << "/" << files.size() << " "
              << schedule_data.back()->name();
    for_each_service(*schedule_data.back(), bitfields, service_builder_fun, c);
  }
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& fbb) {
  auto const core_data_root = hrd_root / CORE_DATA;
  if (fs::is_regular_file(core_data_root / "eckdaten.101")) {
    parse(hrd_root, fbb, hrd_5_00_8_);
  }
  if (fs::is_regular_file(core_data_root / "eckdaten.txt")) {
    parse(hrd_root, fbb, hrd_5_20_26_);
  }
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& fbb,
                       config const& c) {
  auto const core_data_root = hrd_root / CORE_DATA;
  auto const bitfields_file =
      load(core_data_root, BITFIELDS, c.files_.required_files_);
  bitfield_builder bb(parse_bitfields(bitfields_file, c));
  auto const infotext_file =
      load(core_data_root, INFOTEXT, c.files_.required_files_);
  auto const stations_file =
      load(core_data_root, STATIONS, c.files_.required_files_);
  auto const coordinates_file =
      load(core_data_root, COORDINATES, c.files_.required_files_);
  auto const timezones_file =
      load(core_data_root, TIMEZONES, c.files_.required_files_);
  auto const basic_data_file =
      load(core_data_root, BASIC_DATA, c.files_.required_files_);
  auto const footp_file_1 =
      load(core_data_root, FOOTPATHS, c.files_.required_files_);
  station_meta_data metas;
  if (c.version_ == "hrd_5_00_8") {
    auto const footp_file_2 =
        load(core_data_root, FOOTPATHS_EXT, c.files_.required_files_);
    parse_station_meta_data(infotext_file, footp_file_1, footp_file_2, metas,
                            c);
  } else {
    auto const footp_file_2 = loaded_file();
    parse_station_meta_data(infotext_file, footp_file_1, footp_file_2, metas,
                            c);
  }

  station_builder stb(parse_stations(stations_file, coordinates_file, metas, c),
                      parse_timezones(timezones_file, basic_data_file, c));

  auto const categories_file =
      load(core_data_root, CATEGORIES, c.files_.required_files_);
  category_builder cb(parse_categories(categories_file, c));

  auto const providers_file =
      load(core_data_root, PROVIDERS, c.files_.required_files_);
  provider_builder pb(parse_providers(providers_file, c));

  auto const attributes_file =
      load(core_data_root, ATTRIBUTES, c.files_.required_files_);
  attribute_builder ab(parse_attributes(attributes_file, c));

  auto const directions_file =
      load(core_data_root, DIRECTIONS, c.files_.required_files_);
  direction_builder db(parse_directions(directions_file, c));

  auto const tracks_file =
      load(core_data_root, TRACKS, c.files_.required_files_);
  service_builder sb(parse_track_rules(tracks_file, fbb, c));

  line_builder lb;
  route_builder rb;

  auto const ts_file =
      load(core_data_root, THROUGH_SERVICES, c.files_.required_files_);
  auto const mss_file =
      load(core_data_root, MERGE_SPLIT_SERVICES, c.files_.required_files_);
  service_rules rules;
  parse_through_service_rules(ts_file, bb.hrd_bitfields_, rules, c);
  parse_merge_split_service_rules(mss_file, bb.hrd_bitfields_, rules, c);
  rule_service_builder rsb(rules);

  // parse and build services
  std::vector<std::unique_ptr<loaded_file>> schedule_data;
  parse_and_build_services(hrd_root, bb.hrd_bitfields_, schedule_data,
                           [&](hrd_service const& s) {
                             if (!rsb.add_service(s)) {
                               sb.create_service(s, rb, stb, cb, pb, lb, ab, bb,
                                                 db, fbb, false);
                             }
                           },
                           c);

  // compute and build rule services
  rsb.resolve_rule_services();
  rsb.create_rule_services(
      [&](hrd_service const& s, bool is_rule_service, FlatBufferBuilder& fbb) {
        return sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb,
                                 is_rule_service);
      },
      stb, fbb);

  auto interval = parse_interval(basic_data_file);
  auto schedule_name = parse_schedule_name(basic_data_file);
  auto footpaths = create_footpaths(metas.footpaths_, stb, fbb);
  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVectorOfSortedTables(&sb.fbs_services_),
      fbb.CreateVector(values(stb.fbs_stations_)),
      fbb.CreateVector(values(rb.routes_)), &interval, footpaths,
      fbb.CreateVector(rsb.fbs_rule_services_),
      create_meta_stations(metas.meta_stations_, stb.fbs_stations_, fbb),
      fbb.CreateString(schedule_name)));
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
