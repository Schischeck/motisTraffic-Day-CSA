#include <cinttypes>
#include <cstring>

#include "catch/catch.hpp"

#include "test_spec.h"

#include "parser/cstr.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"
#include "motis/loader/parsers/hrd/service/specification.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/util.h"

#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;
using namespace flatbuffers;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_hrd_service_full_range") {
  test_spec services_file(SCHEDULES / "hand-crafted" / "fahrten",
                          "services-1.101");

  auto services = services_file.get_services();
  REQUIRE(services.size() == 1);

  auto const& service = services[0];
  REQUIRE(service.sections_.size() == 5);
  std::for_each(
      std::begin(service.sections_), std::end(service.sections_),
      [](hrd_service::section const& s) {
        REQUIRE(s.traffic_days == std::vector<int>({2687}));
        REQUIRE(s.train_num == 2292);
        REQUIRE(s.admin == "80____");
        REQUIRE(s.attributes == std::vector<hrd_service::attribute>(
                                    {hrd_service::attribute(0, "BT"),
                                     hrd_service::attribute(0, "FR"),
                                     hrd_service::attribute(0, "G ")}));
        REQUIRE(s.category == std::vector<cstr>({"IC "}));
        REQUIRE(s.line_information == std::vector<cstr>({"381  "}));
      });
  REQUIRE(service.stops_.size() == 6);

  auto stop = service.stops_[0];
  REQUIRE(stop.eva_num == 8000096);
  REQUIRE(stop.arr.time == hrd_service::NOT_SET);
  REQUIRE(stop.dep.time == 965);
  REQUIRE(stop.dep.in_out_allowed);

  stop = service.stops_[1];
  REQUIRE(stop.eva_num == 8000156);
  REQUIRE(stop.arr.time == hhmm_to_min(1644));
  REQUIRE(stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hhmm_to_min(1646));
  REQUIRE(stop.dep.in_out_allowed);

  stop = service.stops_[2];
  REQUIRE(stop.eva_num == 8000377);
  REQUIRE(stop.arr.time == hhmm_to_min(1659));
  REQUIRE(!stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hhmm_to_min(1700));
  REQUIRE(!stop.dep.in_out_allowed);

  stop = service.stops_[3];
  REQUIRE(stop.eva_num == 8000031);
  REQUIRE(stop.arr.time == hhmm_to_min(1708));
  REQUIRE(stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hhmm_to_min(1709));
  REQUIRE(stop.dep.in_out_allowed);

  stop = service.stops_[4];
  REQUIRE(stop.eva_num == 8000068);
  REQUIRE(stop.arr.time == hhmm_to_min(1722));
  REQUIRE(stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hhmm_to_min(1724));
  REQUIRE(stop.dep.in_out_allowed);

  stop = service.stops_[5];
  REQUIRE(stop.eva_num == 8000105);
  REQUIRE(stop.arr.time == hhmm_to_min(1740));
  REQUIRE(stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hrd_service::NOT_SET);
}

TEST_CASE("parse_hrd_service_multiple_ranges") {
  test_spec services_file(SCHEDULES / "hand-crafted" / "fahrten",
                          "services-2.101");

  auto services = services_file.get_services();
  REQUIRE(services.size() == 1);

  auto const& service = services[0];
  REQUIRE(service.sections_.size() == 2);

  auto section = service.sections_[0];
  REQUIRE(section.train_num == 2292);
  REQUIRE(section.admin == "80____");
  REQUIRE(section.attributes == std::vector<hrd_service::attribute>(
                                    {hrd_service::attribute(0, "FR"),
                                     hrd_service::attribute(0, "G ")}));
  REQUIRE(section.category == std::vector<cstr>({"IC "}));
  REQUIRE(section.line_information == std::vector<cstr>({"381  "}));
  REQUIRE(section.traffic_days == std::vector<int>({0}));

  section = service.sections_[1];
  REQUIRE(section.train_num == 2293);
  REQUIRE(section.admin == "81____");
  REQUIRE(section.attributes == std::vector<hrd_service::attribute>(
                                    {hrd_service::attribute(1337, "BT")}));
  REQUIRE(section.category == std::vector<cstr>({"IC "}));
  REQUIRE(section.line_information == std::vector<cstr>({"381  "}));
  REQUIRE(section.traffic_days == std::vector<int>({2687}));

  REQUIRE(service.stops_.size() == 3);

  auto stop = service.stops_[0];
  REQUIRE(stop.eva_num == 8000096);
  REQUIRE(stop.arr.time == hrd_service::NOT_SET);
  REQUIRE(stop.dep.time == 965);
  REQUIRE(stop.dep.in_out_allowed);

  stop = service.stops_[1];
  REQUIRE(stop.eva_num == 8000068);
  REQUIRE(stop.arr.time == hhmm_to_min(1722));
  REQUIRE(stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hhmm_to_min(1724));
  REQUIRE(stop.dep.in_out_allowed);

  stop = service.stops_[2];
  REQUIRE(stop.eva_num == 8000105);
  REQUIRE(stop.arr.time == hhmm_to_min(1740));
  REQUIRE(stop.arr.in_out_allowed);
  REQUIRE(stop.dep.time == hrd_service::NOT_SET);
}

TEST_CASE("parse_trains") {
  try {
    const auto stamm = (SCHEDULES / "hand-crafted") / "stamm";
    const auto fahrten = (SCHEDULES / "hand-crafted") / "fahrten";

    const test_spec bitfields_file(stamm, BITFIELDS_FILE);
    auto bitfields = parse_bitfields(bitfields_file.lf_);

    const test_spec attributes_file(stamm, ATTRIBUTES_FILE);
    auto attributes = parse_attributes(attributes_file.lf_);

    FlatBufferBuilder b;

    const test_spec stations_file(stamm, STATIONS_FILE);
    const test_spec coordinates_file(stamm, COORDINATES_FILE);
    auto stations = parse_stations(stations_file.lf_, coordinates_file.lf_, b);

    const test_spec platforms_file(stamm, PLATFORMS_FILE);
    auto platforms = parse_platform_rules(platforms_file.lf_, b);

    const test_spec services_file(fahrten, "services-all.101");
    std::vector<Offset<Service>> services;
    parse_services(services_file.lf_,
                   shared_data(stations, attributes, bitfields, platforms), b,
                   services);

    b.Finish(CreateSchedule(b, b.CreateVector(services),
                            b.CreateVector(values(stations)), {}));

  } catch (parser_error const& e) {
    printf("parser error: %s:%d\n", e.filename, e.line_number);
  }
}

}  // hrd
}  // loader
}  // motis
