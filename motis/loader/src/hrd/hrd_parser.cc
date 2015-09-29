#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <string>

#include "boost/range/iterator_range.hpp"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/schedule_interval_parser.h"
#include "motis/loader/parsers/hrd/directions_parser.h"
#include "motis/loader/parsers/hrd/footpath_builder.h"
#include "motis/loader/parsers/hrd/schedule_interval_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/stations_translator.h"
#include "motis/loader/parsers/hrd/categories_parser.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/providers_parser.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

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
    PROVIDERS_FILE,  THROUGH_SERVICES_FILE, MERGE_SPLIT_RULES_FILE};

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
  auto sd = parse_shared_data(hrd_root, fbb);

  service_builder sb(sd, fbb);
  parse_services_files(hrd_root, sb);

  auto footpaths =
      build_footpaths(sd.metas.footpaths_, sb.stations_.fbs_stations_, fbb);

  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(sb.services_),
      fbb.CreateVector(values(sb.stations_.fbs_stations_)),
      fbb.CreateVector(values(sb.routes_)), &sd.interval, footpaths));
}

shared_data hrd_parser::parse_shared_data(fs::path const& hrd_root,
                                          FlatBufferBuilder& b) {
  auto root = hrd_root / "stamm";
  auto basic_data_buf = load_file(root / BASIC_DATA_FILE);
  auto stations_names_buf = load_file(root / STATIONS_FILE);
  auto stations_coords_buf = load_file(root / COORDINATES_FILE);
  auto infotext_buf = load_file(root / INFOTEXT_FILE);
  auto attributes_buf = load_file(root / ATTRIBUTES_FILE);
  auto bitfields_buf = load_file(root / BITFIELDS_FILE);
  auto platforms_buf = load_file(root / PLATFORMS_FILE);
  auto categories_buf = load_file(root / CATEGORIES_FILE);
  auto directions_buf = load_file(root / DIRECTIONS_FILE);
  auto providers_buf = load_file(root / PROVIDERS_FILE);
  auto through_services_buf = load_file(root / THROUGH_SERVICES_FILE);
  auto merge_split_rules_buf = load_file(root / MERGE_SPLIT_RULES_FILE);

  station_meta_data metas;
  parse_station_meta_data({INFOTEXT_FILE, infotext_buf}, metas);
  return shared_data(
      parse_interval({BASIC_DATA_FILE, basic_data_buf}), metas,
      parse_stations({STATIONS_FILE, stations_names_buf},
                     {COORDINATES_FILE, stations_coords_buf}, metas),
      parse_categories({CATEGORIES_FILE, categories_buf}),
      parse_attributes({ATTRIBUTES_FILE, attributes_buf}),
      parse_bitfields({BITFIELDS_FILE, bitfields_buf}),
      parse_platform_rules({PLATFORMS_FILE, platforms_buf}, b),
      parse_directions({DIRECTIONS_FILE, directions_buf}),
      parse_providers({PROVIDERS_FILE, providers_buf}),
      parse_through_service_rules(
          {THROUGH_SERVICES_FILE, through_services_buf}),
      parse_merge_split_rules({MERGE_SPLIT_RULES_FILE, merge_split_rules_buf}));
}

void hrd_parser::parse_services_files(fs::path const& hrd_root,
                                      service_builder& sb) {
  auto services_files_root = hrd_root / "fahrten";

  std::vector<fs::path> service_files;
  for (auto const& entry : boost::make_iterator_range(
           fs::directory_iterator(services_files_root), {})) {
    if (fs::is_regular(entry.path())) {
      service_files.push_back(entry.path());
    }
  }

  int count = 0;
  for (auto const& service_file : service_files) {
    LOG(info) << "parsing " << ++count << "/" << service_files.size() << " "
              << service_file;
    parse_services_file(service_file, sb);
  }
}

void hrd_parser::parse_services_file(fs::path const& services_file_path,
                                     service_builder& sb) {
  auto services_buf = load_file(services_file_path);
  parse_services({services_file_path.string().c_str(), services_buf},
                 [&](specification const& spec) {
                   try {
                     sb.create_services(hrd_service(spec));
                   } catch (parser_error const& e) {
                     LOG(error) << "skipping bad service at " << e.filename
                                << ":" << e.line_number;
                   }
                 });
}

}  // hrd
}  // loader
}  // motis
