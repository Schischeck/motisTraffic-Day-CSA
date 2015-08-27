#include <cinttypes>
#include <cstring>

#include "gtest/gtest.h"

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

TEST(loader_hrd_services, parse_hrd_service_full_range) {
  test_spec services_file(SCHEDULES / "hand-crafted" / "fahrten",
                          "services-1.101");

  auto services = services_file.get_services();
  ASSERT_TRUE(services.size() == 1);

  auto const& service = services[0];
  ASSERT_TRUE(service.sections_.size() == 5);
  std::for_each(
      std::begin(service.sections_), std::end(service.sections_),
      [](hrd_service::section const& s) {
        ASSERT_TRUE(s.traffic_days == std::vector<int>({2687}));
        ASSERT_TRUE(s.train_num == 2292);
        ASSERT_TRUE(s.admin == "80____");
        ASSERT_TRUE(s.attributes == std::vector<hrd_service::attribute>(
                                        {hrd_service::attribute(0, "BT"),
                                         hrd_service::attribute(0, "FR"),
                                         hrd_service::attribute(0, "G ")}));
        ASSERT_TRUE(s.category == std::vector<cstr>({"IC "}));
        ASSERT_TRUE(s.line_information == std::vector<cstr>({"381  "}));
      });
  ASSERT_TRUE(service.stops_.size() == 6);

  auto stop = service.stops_[0];
  ASSERT_TRUE(stop.eva_num == 8000096);
  ASSERT_TRUE(stop.arr.time == hrd_service::NOT_SET);
  ASSERT_TRUE(stop.dep.time == 965);
  ASSERT_TRUE(stop.dep.in_out_allowed);

  stop = service.stops_[1];
  ASSERT_TRUE(stop.eva_num == 8000156);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1644));
  ASSERT_TRUE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hhmm_to_min(1646));
  ASSERT_TRUE(stop.dep.in_out_allowed);

  stop = service.stops_[2];
  ASSERT_TRUE(stop.eva_num == 8000377);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1659));
  ASSERT_TRUE(!stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hhmm_to_min(1700));
  ASSERT_TRUE(!stop.dep.in_out_allowed);

  stop = service.stops_[3];
  ASSERT_TRUE(stop.eva_num == 8000031);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1708));
  ASSERT_TRUE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hhmm_to_min(1709));
  ASSERT_TRUE(stop.dep.in_out_allowed);

  stop = service.stops_[4];
  ASSERT_TRUE(stop.eva_num == 8000068);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1722));
  ASSERT_TRUE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hhmm_to_min(1724));
  ASSERT_TRUE(stop.dep.in_out_allowed);

  stop = service.stops_[5];
  ASSERT_TRUE(stop.eva_num == 8000105);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1740));
  ASSERT_TRUE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hrd_service::NOT_SET);
}

TEST(loader_hrd_services, parse_hrd_service_multiple_ranges) {
  test_spec services_file(SCHEDULES / "hand-crafted" / "fahrten",
                          "services-2.101");

  auto services = services_file.get_services();
  ASSERT_TRUE(services.size() == 1);

  auto const& service = services[0];
  ASSERT_TRUE(service.sections_.size() == 2);

  auto section = service.sections_[0];
  ASSERT_TRUE(section.train_num == 2292);
  ASSERT_TRUE(section.admin == "80____");
  ASSERT_TRUE(section.attributes == std::vector<hrd_service::attribute>(
                                        {hrd_service::attribute(0, "FR"),
                                         hrd_service::attribute(0, "G ")}));
  ASSERT_TRUE(section.category == std::vector<cstr>({"IC "}));
  ASSERT_TRUE(section.line_information == std::vector<cstr>({"381  "}));
  ASSERT_TRUE(section.traffic_days == std::vector<int>({0}));

  section = service.sections_[1];
  ASSERT_TRUE(section.train_num == 2293);
  ASSERT_TRUE(section.admin == "81____");
  ASSERT_TRUE(section.attributes == std::vector<hrd_service::attribute>(
                                        {hrd_service::attribute(1337, "BT")}));
  ASSERT_TRUE(section.category == std::vector<cstr>({"IC "}));
  ASSERT_TRUE(section.line_information == std::vector<cstr>({"381  "}));
  ASSERT_TRUE(section.traffic_days == std::vector<int>({2687}));

  ASSERT_TRUE(service.stops_.size() == 3);

  auto stop = service.stops_[0];
  ASSERT_TRUE(stop.eva_num == 8000096);
  ASSERT_TRUE(stop.arr.time == hrd_service::NOT_SET);
  ASSERT_TRUE(stop.dep.time == 965);
  ASSERT_TRUE(stop.dep.in_out_allowed);

  stop = service.stops_[1];
  ASSERT_TRUE(stop.eva_num == 8000068);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1722));
  ASSERT_TRUE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hhmm_to_min(1724));
  ASSERT_TRUE(stop.dep.in_out_allowed);

  stop = service.stops_[2];
  ASSERT_TRUE(stop.eva_num == 8000105);
  ASSERT_TRUE(stop.arr.time == hhmm_to_min(1740));
  ASSERT_TRUE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hrd_service::NOT_SET);
}

TEST(loader_hrd_services, parse_trains) {
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
