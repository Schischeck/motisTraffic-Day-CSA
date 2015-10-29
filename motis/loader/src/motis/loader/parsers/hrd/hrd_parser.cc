#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <string>

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
#include "motis/loader/builders/hrd/footpath_builder.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"
#include "motis/loader/builders/hrd/station_builder.h"

using namespace flatbuffers;
using namespace parser;
using namespace motis::logging;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

std::vector<std::string> const required_files = {
    ATTRIBUTES_FILE, STATIONS_FILE,         COORDINATES_FILE,
    BITFIELDS_FILE,  PLATFORMS_FILE,        INFOTEXT_FILE,
    BASIC_DATA_FILE, CATEGORIES_FILE,       DIRECTIONS_FILE,
    PROVIDERS_FILE,  THROUGH_SERVICES_FILE, MERGE_SPLIT_SERVICES_FILE};

bool hrd_parser::applicable(fs::path const& path) {
  auto const master_data_root = path / "stamm";

  return fs::is_directory(path / "fahrten") &&
         std::all_of(begin(required_files), end(required_files),
                     [&master_data_root](std::string const& f) {
                       return fs::is_regular_file(master_data_root / f);
                     });
}

std::vector<std::string> hrd_parser::missing_files(fs::path const& path) const {
  std::vector<std::string> files;
  if (!fs::is_directory(path / "fahrten")) {
    files.push_back((path / "fahrten").string().c_str());
  }
  auto const master_data_root = path / "stamm";
  std::copy_if(
      begin(required_files), end(required_files), std::back_inserter(files),
      [&](std::string const& f) {
        return !fs::is_regular_file((master_data_root / f).string().c_str());
      });
  return files;
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& fbb) {
  // auto shared_data = parse_shared_data(hrd_root, fbb);

  auto stamm_root = hrd_root / "stamm";

  auto bitfields_buf = load_file(stamm_root / BITFIELDS_FILE);
  bitfield_builder bb(parse_bitfields({BITFIELDS_FILE, bitfields_buf}));

  route_builder rb;

  auto stations_names_buf = load_file(stamm_root / STATIONS_FILE);
  auto stations_coords_buf = load_file(stamm_root / COORDINATES_FILE);
  auto infotext_buf = load_file(stamm_root / INFOTEXT_FILE);
  station_meta_data metas;
  parse_station_meta_data({INFOTEXT_FILE, infotext_buf}, metas);
  station_builder stb(parse_stations({STATIONS_FILE, stations_names_buf},
                                     {COORDINATES_FILE, stations_coords_buf},
                                     metas));

  auto categories_buf = load_file(stamm_root / CATEGORIES_FILE);
  category_builder cb(parse_categories({CATEGORIES_FILE, categories_buf}));

  auto providers_buf = load_file(stamm_root / PROVIDERS_FILE);
  provider_builder pb(parse_providers({PROVIDERS_FILE, providers_buf}));

  line_builder lb;

  auto attributes_buf = load_file(stamm_root / ATTRIBUTES_FILE);
  attribute_builder ab(parse_attributes({ATTRIBUTES_FILE, attributes_buf}));

  auto directions_buf = load_file(stamm_root / DIRECTIONS_FILE);
  direction_builder db(parse_directions({DIRECTIONS_FILE, directions_buf}));

  auto platforms_buf = load_file(stamm_root / PLATFORMS_FILE);
  service_builder sb(
      parse_platform_rules({PLATFORMS_FILE, platforms_buf}, fbb));

  service_rules rules;
  auto through_services_buf = load_file(stamm_root / THROUGH_SERVICES_FILE);
  parse_through_service_rules({THROUGH_SERVICES_FILE, through_services_buf},
                              bb.hrd_bitfields_, rules);
  auto merge_split_rules_buf =
      load_file(stamm_root / MERGE_SPLIT_SERVICES_FILE);
  parse_merge_split_service_rules(
      {MERGE_SPLIT_SERVICES_FILE, merge_split_rules_buf}, bb.hrd_bitfields_,
      rules);
  rule_service_builder rsb(rules);

  // collect services files
  auto services_files_root = hrd_root / "fahrten";
  std::vector<fs::path> service_files;
  for (auto const& entry : boost::make_iterator_range(
           fs::directory_iterator(services_files_root), {})) {
    if (fs::is_regular(entry.path())) {
      service_files.push_back(entry.path());
    }
  }

  // parse and export services
  int count = 0;
  for (auto const& service_file : service_files) {
    LOG(info) << "parsing " << ++count << "/" << service_files.size() << " "
              << service_file;
    auto buf = load_file(service_file);
    for_each_service({service_file.string().c_str(), buf}, bb.hrd_bitfields_,
                     [&](hrd_service const& s) {
                       if (!rsb.add_service(s)) {
                         sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db,
                                           fbb);
                       }
                     });
  }
  rsb.resolve_rule_services();
  rsb.create_rule_services([&](hrd_service const& s, FlatBufferBuilder& fbb) {
    sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
    return sb.fbs_services_.back();
  }, fbb);

  auto basic_data_buf = load_file(stamm_root / BASIC_DATA_FILE);
  auto footpaths = create_footpaths(metas.footpaths_, stb.fbs_stations_, fbb);
  auto interval = parse_interval({BASIC_DATA_FILE, basic_data_buf});
  fbb.Finish(CreateSchedule(fbb, fbb.CreateVector(sb.fbs_services_),
                            fbb.CreateVector(values(stb.fbs_stations_)),
                            fbb.CreateVector(values(rb.routes_)), &interval,
                            footpaths,
                            fbb.CreateVector(rsb.fbs_rule_services)));
}

}  // hrd
}  // loader
}  // motis
