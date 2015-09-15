#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <string>

#include "boost/range/iterator_range.hpp"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/directions_parser.h"
#include "motis/loader/parsers/hrd/footpath_builder.h"
#include "motis/loader/parsers/hrd/schedule_interval_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/stations_translator.h"
#include "motis/loader/parsers/hrd/categories_parser.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/providers_parser.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace flatbuffers;
using namespace parser;
using namespace motis::logging;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

std::vector<std::string> const required_files = {
    ATTRIBUTES_FILE, STATIONS_FILE, COORDINATES_FILE, BITFIELDS_FILE,
    PLATFORMS_FILE,  INFOTEXT_FILE, BASIC_DATA_FILE,  CATEGORIES_FILE,
    DIRECTIONS_FILE, PROVIDERS_FILE};

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
  auto data = parse_shared_data(hrd_root, fbb);
  auto const& sd = std::get<0>(data);
  auto const& interval = std::get<1>(data);
  auto const& metas = std::get<2>(data);

  service_builder sb(sd, fbb);
  parse_services_files(hrd_root, sb);

  auto footpaths =
      build_footpaths(metas.footpaths_, sb.stations_.fbs_stations_, fbb);

  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(sb.services_),
      fbb.CreateVector(values(sb.stations_.fbs_stations_)),
      fbb.CreateVector(values(sb.routes_)), &interval, footpaths));
}

std::tuple<shared_data, Interval, station_meta_data>
hrd_parser::parse_shared_data(fs::path const& hrd_root, FlatBufferBuilder& b) {
  auto master_data_root = hrd_root / "stamm";
  auto stations_names_buf = load_file(master_data_root / STATIONS_FILE);
  auto stations_coords_buf = load_file(master_data_root / COORDINATES_FILE);
  auto infotext_buf = load_file(master_data_root / INFOTEXT_FILE);
  auto attributes_buf = load_file(master_data_root / ATTRIBUTES_FILE);
  auto bitfields_buf = load_file(master_data_root / BITFIELDS_FILE);
  auto platforms_buf = load_file(master_data_root / PLATFORMS_FILE);
  auto categories_buf = load_file(master_data_root / CATEGORIES_FILE);
  auto directions_buf = load_file(master_data_root / DIRECTIONS_FILE);
  auto providers_buf = load_file(master_data_root / PROVIDERS_FILE);

  station_meta_data metas;
  parse_station_meta_data({INFOTEXT_FILE, infotext_buf}, metas);

  shared_data sd(parse_stations({STATIONS_FILE, stations_names_buf},
                                {COORDINATES_FILE, stations_coords_buf}, metas),
                 parse_categories({CATEGORIES_FILE, categories_buf}),
                 parse_attributes({ATTRIBUTES_FILE, attributes_buf}),
                 parse_bitfields({BITFIELDS_FILE, bitfields_buf}),
                 parse_platform_rules({PLATFORMS_FILE, platforms_buf}, b),
                 parse_directions({DIRECTIONS_FILE, directions_buf}),
                 parse_providers({PROVIDERS_FILE, providers_buf}));
  auto basic_data_buf = load_file(master_data_root / BASIC_DATA_FILE);
  return std::make_tuple(std::move(sd),
                         parse_interval({BASIC_DATA_FILE, basic_data_buf}),
                         std::move(metas));
}

void hrd_parser::parse_services_files(fs::path const& hrd_root,
                                      service_builder& sb) {
  auto services_files_root = hrd_root / "fahrten";
  for (auto const& entry : boost::make_iterator_range(
           fs::directory_iterator(services_files_root), {})) {
    auto const& services_file_path = entry.path();

    if (fs::is_regular(services_file_path)) {
      LOG(info) << "parsing " << services_file_path;
      parse_services_file(services_file_path, sb);
    }
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
