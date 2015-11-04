#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <string>
#include <stack>

#include "boost/range/iterator_range.hpp"

#include "parser/file.h"

#include "motis/core/common/logging.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/schedule_interval_parser.h"
#include "motis/loader/parsers/hrd/directions_parser.h"
#include "motis/loader/parsers/hrd/schedule_interval_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/categories_parser.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/providers_parser.h"
#include "motis/loader/parsers/hrd/service_parser.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"
#include "motis/loader/builders/hrd/attribute_builder.h"
#include "motis/loader/builders/hrd/bitfield_builder.h"
#include "motis/loader/builders/hrd/category_builder.h"
#include "motis/loader/builders/hrd/direction_builder.h"
#include "motis/loader/builders/hrd/footpath_builder.h"
#include "motis/loader/builders/hrd/line_builder.h"
#include "motis/loader/builders/hrd/provider_builder.h"
#include "motis/loader/builders/hrd/route_builder.h"
#include "motis/loader/builders/hrd/rule_service_builder.h"
#include "motis/loader/builders/hrd/service_builder.h"
#include "motis/loader/builders/hrd/station_builder.h"

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
  MERGE_SPLIT_SERVICES
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
    {MERGE_SPLIT_SERVICES_FILE}};

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

void collect_services_filenames(fs::path const& root,
                                std::vector<fs::path>& services_files) {
  for (auto const& entry :
       boost::make_iterator_range(fs::directory_iterator(root), {})) {
    if (fs::is_regular(entry.path())) {
      services_files.push_back(entry.path().filename());
    }
  }
}

void parse_and_build_services(
    fs::path const& hrd_root, std::map<int, bitfield> const& bitfields,
    std::vector<loaded_file>& schedule_data,
    std::function<void(hrd_service const&)> service_builder_fun) {
  auto const schedule_data_root = hrd_root / SCHEDULE_DATA;
  std::vector<fs::path> services_filenames;
  collect_services_filenames(schedule_data_root, services_filenames);

  int count = 0;
  for (auto const& filename : services_filenames) {
    auto const services_file = schedule_data_root / filename;
    schedule_data.emplace_back(services_file);
    LOG(info) << "parsing " << ++count << "/" << services_filenames.size()
              << " " << services_file;
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
  station_meta_data metas;
  parse_station_meta_data(infotext_file, metas);
  station_builder stb(parse_stations(stations_file, coordinates_file, metas));

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
        /* TODO fix unbounded layers problem (Tobias Raffel)
         if (!rsb.add_service(s)) {
           sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
         }
         */
        sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
      });

  /* TODO fix unbounded layers problem (Tobias Raffel)
  //compute and build ruleservices
  rsb.resolve_rule_services();
  rsb.create_rule_services([&](hrd_service const& s, FlatBufferBuilder& fbb) {
    sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
    return sb.fbs_services_.back();
  }, fbb);
  */

  auto interval = parse_interval(load(core_data_root, BASIC_DATA));
  fbb.Finish(
      CreateSchedule(fbb, fbb.CreateVector(sb.fbs_services_),
                     fbb.CreateVector(values(stb.fbs_stations_)),
                     fbb.CreateVector(values(rb.routes_)), &interval,
                     create_footpaths(metas.footpaths_, stb.fbs_stations_, fbb),
                     /* fbb.CreateVector(rsb.fbs_rule_services) */ {}));
}

}  // hrd
}  // loader
}  // motis
