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
#include "motis/loader/builders/hrd/footpath_builder.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"
#include "motis/loader/builders/hrd/station_builder.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;
using namespace parser;
using namespace motis::logging;
namespace fs = boost::filesystem;

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

struct hrd_buffer {
  loaded_file push_and_get(fs::path root, char const* filename) {
    buffers_.push(load_file(root / filename));
    return {filename, buffers_.top()};
  }
  void pop(int times = 1) {
    verify(times > 0, "expected #pop > 0, but was %d", times);
    for (int i = 0; !buffers_.empty() && i < times; ++i) {
      buffers_.pop();
    }
  }
  std::stack<buffer> buffers_;
};

void collect_services_filenames(fs::path const& root,
                                std::vector<fs::path>& services_files) {
  for (auto const& entry :
       boost::make_iterator_range(fs::directory_iterator(root), {})) {
    if (fs::is_regular(entry.path())) {
      services_files.push_back(entry.path().filename());
    }
  }
}

void hrd_parser::parse(fs::path const& hrd_root, FlatBufferBuilder& fbb) {
  auto stamm_root = hrd_root / "stamm";
  hrd_buffer hrd;

  // parse service dependencies and create builders
  route_builder rb;
  bitfield_builder bb(
      parse_bitfields(hrd.push_and_get(stamm_root, BITFIELDS_FILE)));
  hrd.pop();
  station_meta_data metas;
  parse_station_meta_data(hrd.push_and_get(stamm_root, INFOTEXT_FILE), metas);
  station_builder stb(
      parse_stations(hrd.push_and_get(stamm_root, STATIONS_FILE),
                     hrd.push_and_get(stamm_root, COORDINATES_FILE), metas));
  hrd.pop(2);
  category_builder cb(
      parse_categories(hrd.push_and_get(stamm_root, CATEGORIES_FILE)));
  hrd.pop();
  provider_builder pb(
      parse_providers(hrd.push_and_get(stamm_root, PROVIDERS_FILE)));
  hrd.pop();
  line_builder lb;
  attribute_builder ab(
      parse_attributes(hrd.push_and_get(stamm_root, ATTRIBUTES_FILE)));
  hrd.pop();
  direction_builder db(
      parse_directions(hrd.push_and_get(stamm_root, DIRECTIONS_FILE)));
  hrd.pop();
  service_builder sb(
      parse_platform_rules(hrd.push_and_get(stamm_root, PLATFORMS_FILE), fbb));
  hrd.pop();
  service_rules rules;
  parse_through_service_rules(
      hrd.push_and_get(stamm_root, THROUGH_SERVICES_FILE), bb.hrd_bitfields_,
      rules);
  hrd.pop();
  parse_merge_split_service_rules(
      hrd.push_and_get(stamm_root, MERGE_SPLIT_SERVICES_FILE),
      bb.hrd_bitfields_, rules);
  hrd.pop();
  rule_service_builder rsb(rules);

  auto services_files_root = hrd_root / "fahrten";
  std::vector<fs::path> services_filenames;
  collect_services_filenames(services_files_root, services_filenames);

  // parse and export services
  int num_services = 0;
  auto builder_fun = [&](hrd_service const& s) {
    bool lower_bound =
        15300 <= s.sections_[0].train_num && s.sections_[0].train_num <= 15400;
    bool upper_bound =
        18260 <= s.sections_[0].train_num && s.sections_[0].train_num <= 18500;
    bool mid =
        18270 <= s.sections_[0].train_num && s.sections_[0].train_num <= 18400;
    if ((lower_bound || upper_bound) && !mid) {
      if (!rsb.add_service(s)) {
        sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
      } else {
        ++num_services;
        if (lower_bound) {
          LOG(debug) "lower_bound: service at: " << s.origin_.line_number;
        }
        if (upper_bound) {
          LOG(debug) "upper_bound: service at: " << s.origin_.line_number;
        }
      }
    }
  };

  int count = 0;
  for (auto const& filename : services_filenames) {
    LOG(info) << "parsing " << ++count << "/" << services_filenames.size()
              << " " << services_files_root / filename;
    auto const& lf =
        hrd.push_and_get(services_files_root, filename.string().c_str());
    for_each_service(lf, bb.hrd_bitfields_, builder_fun);
  }

  LOG(info) << "#services: " << num_services;
  // compute and export rule services
  rsb.resolve_rule_services();
  rsb.create_rule_services([&](hrd_service const& s, FlatBufferBuilder& fbb) {
    sb.create_service(s, rb, stb, cb, pb, lb, ab, bb, db, fbb);
    return sb.fbs_services_.back();
  }, fbb);
  hrd.pop(services_filenames.size());
  hrd.pop();  // infotext.101

  auto interval = parse_interval(hrd.push_and_get(stamm_root, BASIC_DATA_FILE));
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
