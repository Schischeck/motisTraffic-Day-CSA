#include "motis/loader/hrd/hrd_parser.h"

#include "motis/core/common/logging.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/hrd/files.h"
#include "motis/loader/hrd/parser/schedule_interval_parser.h"
#include "motis/loader/hrd/parser/directions_parser.h"
#include "motis/loader/hrd/parser/schedule_interval_parser.h"
#include "motis/loader/hrd/parser/stations_parser.h"
#include "motis/loader/hrd/parser/categories_parser.h"
#include "motis/loader/hrd/parser/attributes_parser.h"
#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/hrd/parser/platform_rules_parser.h"
#include "motis/loader/hrd/parser/providers_parser.h"
#include "motis/loader/hrd/parser/service_parser.h"
#include "motis/loader/hrd/parser/through_services_parser.h"
#include "motis/loader/hrd/parser/merge_split_rules_parser.h"
#include "motis/loader/hrd/parser/timezones_parser.h"
#include "motis/loader/hrd/builder/attribute_builder.h"
#include "motis/loader/hrd/builder/bitfield_builder.h"
#include "motis/loader/hrd/builder/category_builder.h"
#include "motis/loader/hrd/builder/direction_builder.h"
#include "motis/loader/hrd/builder/footpath_builder.h"
#include "motis/loader/hrd/builder/line_builder.h"
#include "motis/loader/hrd/builder/provider_builder.h"
#include "motis/loader/hrd/builder/route_builder.h"
#include "motis/loader/hrd/builder/rule_service_builder.h"
#include "motis/loader/hrd/builder/service_builder.h"
#include "motis/loader/hrd/builder/station_builder.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;
using namespace parser;
using namespace motis::logging;
namespace fs = boost::filesystem;

enum filename_key {
  ATTRIBUTES,
  STATIONS,
  COORDINATES,
  BITFIELDS,
  PLATFORMS,
  INFOTEXT,
  BASIC_DATA,
  CATEGORIES,
  DIRECTIONS,
  PROVIDERS,
  THROUGH_SERVICES,
  MERGE_SPLIT_SERVICES,
  TIMEZONES,
  FOOTPATHS_REG,
  FOOTPATHS_EXT
};

std::vector<std::vector<std::string>> const required_files = {
    {ATTRIBUTES_FILE_OLD, ATTRIBUTES_FILE_NEW},
    {STATIONS_FILE},
    {COORDINATES_FILE},
    {BITFIELDS_FILE},
    {PLATFORMS_FILE},
    {INFOTEXT_FILE},
    {BASIC_DATA_FILE},
    {CATEGORIES_FILE},
    {DIRECTIONS_FILE},
    {PROVIDERS_FILE},
    {THROUGH_SERVICES_FILE},
    {MERGE_SPLIT_SERVICES_FILE},
    {TIMEZONES_FILE},
    {FOOTPATHS_REG_FILE},
    {FOOTPATHS_EXT_FILE}};

bool hrd_parser::applicable(fs::path const& path) {
  auto const core_data_root = path / CORE_DATA;
  return fs::is_directory(path / SCHEDULE_DATA) &&
         std::all_of(
             begin(required_files), end(required_files),
             [&core_data_root](std::vector<std::string> const& alternatives) {
               return std::any_of(
                   begin(alternatives), end(alternatives),
                   [&core_data_root](std::string const& filename) {
                     return fs::is_regular_file(core_data_root / filename);
                   });
             });
}

std::vector<std::string> hrd_parser::missing_files(
    fs::path const& hrd_root) const {
  std::vector<std::string> missing_files;
  auto const schedule_data_root = hrd_root / SCHEDULE_DATA;
  if (!fs::is_directory(schedule_data_root)) {
    missing_files.push_back(schedule_data_root.string().c_str());
  }
  auto const core_data_root = hrd_root / CORE_DATA;
  for (auto const& alternatives : required_files) {
    std::copy_if(begin(alternatives), end(alternatives),
                 std::back_inserter(missing_files),
                 [&core_data_root](std::string const& filename) {
                   return fs::is_regular_file(
                       (core_data_root / filename).string().c_str());
                 });
  }
  return missing_files;
}

loaded_file load(fs::path const& root, filename_key k) {
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
    std::vector<loaded_file>& schedule_data,
    std::function<void(hrd_service const&)> service_builder_fun) {
  auto const schedule_data_root = hrd_root / SCHEDULE_DATA;

  std::vector<fs::path> files;
  collect_files(schedule_data_root, files);

  int count = 0;
  for (auto const& file : files) {
    schedule_data.emplace_back(file);
    LOG(info) << "parsing " << ++count << "/" << files.size() << " "
              << schedule_data.back().name();
    for_each_service(schedule_data.back(), bitfields, service_builder_fun);
  }
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& fbb) {
  auto const core_data_root = hrd_root / CORE_DATA;

  auto const bitfields_file = load(core_data_root, BITFIELDS);
  bitfield_builder bb(parse_bitfields(bitfields_file));

  auto const infotext_file = load(core_data_root, INFOTEXT);
  auto const stations_file = load(core_data_root, STATIONS);
  auto const coordinates_file = load(core_data_root, COORDINATES);
  auto const timezones_file = load(core_data_root, TIMEZONES);
  auto const basic_data_file = load(core_data_root, BASIC_DATA);
  auto const footp_1_file = load(core_data_root, FOOTPATHS_REG);
  auto const footp_2_file = load(core_data_root, FOOTPATHS_EXT);
  station_meta_data metas;
  parse_station_meta_data(infotext_file, footp_1_file, footp_2_file, metas);
  station_builder stb(parse_stations(stations_file, coordinates_file, metas),
                      parse_timezones(timezones_file, basic_data_file));

  auto const categories_file = load(core_data_root, CATEGORIES);
  category_builder cb(parse_categories(categories_file));

  auto const providers_file = load(core_data_root, PROVIDERS);
  provider_builder pb(parse_providers(providers_file));

  auto const attributes_file = load(core_data_root, ATTRIBUTES);
  attribute_builder ab(parse_attributes(attributes_file));

  auto const directions_file = load(core_data_root, DIRECTIONS);
  direction_builder db(parse_directions(directions_file));

  auto const platforms_file = load(core_data_root, PLATFORMS);
  service_builder sb(parse_platform_rules(platforms_file, fbb));

  line_builder lb;
  route_builder rb;

  auto const ts_file = load(core_data_root, THROUGH_SERVICES);
  auto const mss_file = load(core_data_root, MERGE_SPLIT_SERVICES);
  service_rules rules;
  parse_through_service_rules(ts_file, bb.hrd_bitfields_, rules);
  parse_merge_split_service_rules(mss_file, bb.hrd_bitfields_, rules);
  rule_service_builder rsb(rules);

  // parse and build services
  std::vector<loaded_file> schedule_data;
  parse_and_build_services(
      hrd_root, bb.hrd_bitfields_, schedule_data, [&](hrd_service const& s) {
        if (!rsb.add_service(s)) {
          sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
        }
      });

  // compute and build ruleservices
  rsb.resolve_rule_services();
  rsb.create_rule_services([&](hrd_service const& s, FlatBufferBuilder& fbb) {
    sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
    return sb.fbs_services_.back();
  }, fbb);

  auto interval = parse_interval(basic_data_file);
  fbb.Finish(
      CreateSchedule(fbb, fbb.CreateVector(sb.fbs_services_),
                     fbb.CreateVector(values(stb.fbs_stations_)),
                     fbb.CreateVector(values(rb.routes_)), &interval,
                     create_footpaths(metas.footpaths_, stb.fbs_stations_, fbb),
                     fbb.CreateVector(rsb.fbs_rule_services)));
}

}  // hrd
}  // loader
}  // motis
