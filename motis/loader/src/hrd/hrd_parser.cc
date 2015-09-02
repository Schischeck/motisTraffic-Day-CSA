#include "motis/loader/parsers/hrd/hrd_parser.h"

#include <vector>
#include <string>

#include "boost/filesystem.hpp"
#include "boost/range/iterator_range.hpp"

#include "parser/file.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/service/shared_data.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"
#include "motis/loader/parsers/hrd/service/service_builder.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

bool hrd_parser::applicable(fs::path const& path) {
  if (!fs::is_directory(path / "fahrten")) {
    return false;
  }

  std::vector<std::string> file_names = {
      ATTRIBUTES_FILE, STATIONS_FILE,  COORDINATES_FILE,
      BITFIELDS_FILE,  PLATFORMS_FILE, INFOTEXT_FILE,
  };
  for (auto const& file_name : file_names) {
    if (!fs::is_regular_file(path / "stamm" / file_name)) {
      return false;
    }
  }

  return true;
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& b) {
  auto stamm_path = hrd_root / "stamm";
  auto fahrten_path = hrd_root / "fahrten";

  auto stations_names_buf = load_file(stamm_path / STATIONS_FILE);
  auto stations_coords_buf = load_file(stamm_path / COORDINATES_FILE);
  auto infotext_buf = load_file(stamm_path / INFOTEXT_FILE);
  auto attributes_buf = load_file(stamm_path / ATTRIBUTES_FILE);
  auto bitfields_buf = load_file(stamm_path / BITFIELDS_FILE);
  auto platforms_buf = load_file(stamm_path / PLATFORMS_FILE);

  shared_data stamm(parse_stations({STATIONS_FILE, stations_names_buf},
                                   {COORDINATES_FILE, stations_coords_buf},
                                   {INFOTEXT_FILE, infotext_buf}, b),
                    parse_attributes({ATTRIBUTES_FILE, attributes_buf}),
                    parse_bitfields({BITFIELDS_FILE, bitfields_buf}),
                    parse_platform_rules({PLATFORMS_FILE, platforms_buf}, b));

  service_builder sb(stamm, b);

  std::vector<flatbuffers::Offset<Service>> services;
  for (auto const& entry :
       boost::make_iterator_range(fs::directory_iterator(fahrten_path), {})) {
    auto const& services_file_path = entry.path();
    if (fs::is_regular(services_file_path)) {
      auto services_buf = load_file(services_file_path);
      parse_services({services_file_path.string().c_str(), services_buf},
                     [&b, &sb, &services](specification const& spec) {
                       sb.create_services(hrd_service(spec), b, services);
                     });
    }
  }

  b.Finish(CreateSchedule(b, b.CreateVector(services),
                          b.CreateVector(values(stamm.stations)),
                          b.CreateVector(values(sb.routes_))));
}

}  // hrd
}  // loader
}  // motis
