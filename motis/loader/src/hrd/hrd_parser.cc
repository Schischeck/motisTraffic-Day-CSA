#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <string>

#include "boost/range/iterator_range.hpp"

#include "parser/file.h"
#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/schedule_interval_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/categories_parser.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

bool hrd_parser::applicable(fs::path const& path) {
  auto const master_data_root = path / "stamm";
  std::vector<std::string> const filenames = {
      ATTRIBUTES_FILE, STATIONS_FILE, COORDINATES_FILE, BITFIELDS_FILE,
      PLATFORMS_FILE,  INFOTEXT_FILE, BASIC_DATA_FILE,  CATEGORIES_FILE};

  return fs::is_directory(path / "fahrten") &&
         std::all_of(begin(filenames), end(filenames),
                     [&master_data_root](std::string const& f) {
                       return fs::is_regular_file(master_data_root / f);
                     });
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& fbb) {
  auto sd = parse_shared_data(hrd_root, fbb);
  auto services_data = parse_services_files(hrd_root, sd.first, fbb);

  fbb.Finish(CreateSchedule(fbb, fbb.CreateVector(services_data.first),
                            fbb.CreateVector(values(sd.first.stations)),
                            fbb.CreateVector(services_data.second),
                            &sd.second));
}

std::pair<shared_data, Interval> hrd_parser::parse_shared_data(
    fs::path const& hrd_root, FlatBufferBuilder& b) {
  auto master_data_root = hrd_root / "stamm";
  auto stations_names_buf = load_file(master_data_root / STATIONS_FILE);
  auto stations_coords_buf = load_file(master_data_root / COORDINATES_FILE);
  auto infotext_buf = load_file(master_data_root / INFOTEXT_FILE);
  auto attributes_buf = load_file(master_data_root / ATTRIBUTES_FILE);
  auto bitfields_buf = load_file(master_data_root / BITFIELDS_FILE);
  auto platforms_buf = load_file(master_data_root / PLATFORMS_FILE);
  auto categories_buf = load_file(master_data_root / CATEGORIES_FILE);

  station_meta_data metas;
  parse_station_meta_data({INFOTEXT_FILE, infotext_buf}, metas);

  shared_data sd(
      parse_stations({STATIONS_FILE, stations_names_buf},
                     {COORDINATES_FILE, stations_coords_buf}, metas, b),
      parse_categories({CATEGORIES_FILE, categories_buf}),
      parse_attributes({ATTRIBUTES_FILE, attributes_buf}),
      parse_bitfields({BITFIELDS_FILE, bitfields_buf}),
      parse_platform_rules({PLATFORMS_FILE, platforms_buf}, b));

  auto basic_data_buf = load_file(master_data_root / BASIC_DATA_FILE);
  auto interval = parse_interval({BASIC_DATA_FILE, basic_data_buf});

  return std::make_pair(std::move(sd), interval);
}

std::pair<std::vector<Offset<Service>>, std::vector<Offset<Route>>>
hrd_parser::parse_services_files(fs::path const& hrd_root,
                                 shared_data const& master_data,
                                 FlatBufferBuilder& fbb) {
  auto services_files_root = hrd_root / "fahrten";

  service_builder sb(master_data, fbb);
  std::vector<Offset<Service>> services;
  for (auto const& entry : boost::make_iterator_range(
           fs::directory_iterator(services_files_root), {})) {
    auto const& services_file_path = entry.path();

    if (fs::is_regular(services_file_path)) {
      parse_services_file(services_file_path, services, sb, fbb);
    }
  }
  return std::make_pair(std::move(services), values(sb.routes_));
}

void hrd_parser::parse_services_file(
    fs::path const& services_file_path,
    std::vector<flatbuffers::Offset<Service>>& services, service_builder& sb,
    FlatBufferBuilder& fbb) {
  auto services_buf = load_file(services_file_path);
  parse_services({services_file_path.string().c_str(), services_buf},
                 [&](specification const& spec) {
                   sb.create_services(hrd_service(spec), fbb, services);
                 });
}

}  // hrd
}  // loader
}  // motis
