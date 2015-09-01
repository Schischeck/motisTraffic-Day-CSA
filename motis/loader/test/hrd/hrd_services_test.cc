#include <cinttypes>
#include <cstring>
#include <iostream>

#include "gtest/gtest.h"

#include "test_spec.h"

#include "parser/cstr.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/hrd_parser.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/service/service_parser.h"
#include "motis/loader/parsers/hrd/service/specification.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/service/service_builder.h"
#include "motis/loader/util.h"

#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;
using namespace flatbuffers;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_hrd_services, parse_hrd_service_full_range) {
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

TEST(loader_hrd_hrd_services, parse_hrd_service_multiple_ranges) {
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

TEST(loader_hrd_hrd_services, index_bus) {
  test_spec services_file(SCHEDULES / "single-index-bus" / "fahrten",
                          "services.101");

  auto services = services_file.get_services();
  ASSERT_TRUE(services.size() == 1);

  auto const& service = services[0];
  ASSERT_TRUE(service.sections_.size() == 51);

  std::for_each(
      std::begin(service.sections_), std::end(service.sections_),
      [](hrd_service::section const& section) {
        ASSERT_TRUE(section.train_num == 0);
        ASSERT_TRUE(section.admin == "rmvNOL");
        ASSERT_TRUE(section.attributes ==
                    std::vector<hrd_service::attribute>(
                        {hrd_service::attribute(0, "OB")}));
        ASSERT_TRUE(section.category == std::vector<cstr>({"Bus"}));
        ASSERT_TRUE(section.line_information == std::vector<cstr>({"84/85"}));
        ASSERT_TRUE(section.traffic_days == std::vector<int>({2310}));
      });
}

TEST(loader_hrd_hrd_services, parse_trains) {
  const auto stamm_path = (SCHEDULES / "hand-crafted") / "stamm";
  const auto fahrten_path = (SCHEDULES / "hand-crafted") / "fahrten";

  const test_spec bitfields_file(stamm_path, BITFIELDS_FILE);
  auto bitfields = parse_bitfields(bitfields_file.lf_);

  const test_spec attributes_file(stamm_path, ATTRIBUTES_FILE);
  auto attributes = parse_attributes(attributes_file.lf_);

  FlatBufferBuilder b;

  const test_spec stations_file(stamm_path, STATIONS_FILE);
  const test_spec coordinates_file(stamm_path, COORDINATES_FILE);
  const test_spec infotext_file(stamm_path, INFOTEXT_FILE);
  auto stations = parse_stations(stations_file.lf_, coordinates_file.lf_,
                                 infotext_file.lf_, b);

  const test_spec platforms_file(stamm_path, PLATFORMS_FILE);
  auto platforms = parse_platform_rules(platforms_file.lf_, b);

  const test_spec services_file(fahrten_path, "services-all.101");
  std::vector<Offset<Service>> services;

  shared_data stamm(stations, attributes, bitfields, platforms);
  service_builder sb(stamm, b);
  parse_services(services_file.lf_,
                 [&b, &sb, &services](specification const& spec) {
                   sb.create_services(hrd_service(spec), b, services);
                 });
}

TEST(loader_hrd_hrd_services, parse_trains_ice) {
  const auto stamm_path = SCHEDULES / "multiple-ice-services" / "stamm";
  const auto fahrten_path = SCHEDULES / "multiple-ice-services" / "fahrten";

  const test_spec bitfields_file(stamm_path, BITFIELDS_FILE);
  auto bitfields = parse_bitfields(bitfields_file.lf_);

  const test_spec attributes_file(stamm_path, ATTRIBUTES_FILE);
  auto attributes = parse_attributes(attributes_file.lf_);

  FlatBufferBuilder b;

  const test_spec stations_file(stamm_path, STATIONS_FILE);
  const test_spec coordinates_file(stamm_path, COORDINATES_FILE);
  const test_spec infotext_file(stamm_path, INFOTEXT_FILE);
  auto stations = parse_stations(stations_file.lf_, coordinates_file.lf_,
                                 infotext_file.lf_, b);

  const test_spec platforms_file(stamm_path, PLATFORMS_FILE);
  auto platforms = parse_platform_rules(platforms_file.lf_, b);

  const test_spec services_file(fahrten_path, "services.101");
  std::vector<Offset<Service>> services;

  shared_data stamm(stations, attributes, bitfields, platforms);
  service_builder sb(stamm, b);
  parse_services(services_file.lf_,
                 [&b, &sb, &services](specification const& spec) {
                   sb.create_services(hrd_service(spec), b, services);
                 });
}

TEST(loader_hrd_hrd_services, parse_repetition_service) {
  const auto stamm_path = SCHEDULES / "single-ice" / "stamm";
  const auto fahrten_path = SCHEDULES / "single-ice" / "fahrten";

  const test_spec bitfields_file(stamm_path, BITFIELDS_FILE);
  auto bitfields = parse_bitfields(bitfields_file.lf_);

  const test_spec attributes_file(stamm_path, ATTRIBUTES_FILE);
  auto attributes = parse_attributes(attributes_file.lf_);

  FlatBufferBuilder b;

  const test_spec stations_file(stamm_path, STATIONS_FILE);
  const test_spec coordinates_file(stamm_path, COORDINATES_FILE);
  const test_spec infotext_file(stamm_path, INFOTEXT_FILE);
  auto stations = parse_stations(stations_file.lf_, coordinates_file.lf_,
                                 infotext_file.lf_, b);

  const test_spec platforms_file(stamm_path, PLATFORMS_FILE);
  auto platforms = parse_platform_rules(platforms_file.lf_, b);

  const test_spec services_file(fahrten_path, "repeated_service.101");

  shared_data stamm(stations, attributes, bitfields, platforms);
  service_builder sb(stamm, b);

  std::vector<Offset<Service>> services;
  parse_services(services_file.lf_,
                 [&b, &sb, &services](specification const& spec) {
                   sb.create_services(hrd_service(spec), b, services);
                 });

  ASSERT_TRUE(services.size() == 3);

  b.Finish(CreateSchedule(b, b.CreateVector(services),
                          b.CreateVector(values(stamm.stations)),
                          b.CreateVector(values(sb.routes_))));

  auto schedule = GetSchedule(b.GetBufferPointer());

  ASSERT_TRUE(schedule->services()->size() == 3);

  auto service1 = schedule->services()->Get(0);
  ASSERT_TRUE(service1->times()->size() == 4);
  ASSERT_TRUE(service1->times()->Get(0) == -1);
  ASSERT_TRUE(service1->times()->Get(1) == 1059);
  ASSERT_TRUE(service1->times()->Get(2) == 1388);
  ASSERT_TRUE(service1->times()->Get(3) == -1);
  ASSERT_TRUE(service1->sections()->size() == 1);
  ASSERT_TRUE(
      !deserialize_bitset<512>(service1->traffic_days()->c_str()).any());
  ASSERT_TRUE(service1->sections()->Get(0)->category()->str() == "ICE");
  ASSERT_TRUE(service1->platforms()->size() == 2);
  ASSERT_TRUE(service1->platforms()->Get(0)->arr_platforms()->size() == 0);
  ASSERT_TRUE(service1->platforms()->Get(0)->dep_platforms()->size() == 2);
  ASSERT_STREQ(
      service1->platforms()->Get(0)->dep_platforms()->Get(0)->name()->c_str(),
      "15");
  ASSERT_STREQ(
      service1->platforms()->Get(0)->dep_platforms()->Get(1)->name()->c_str(),
      "18");
  ASSERT_TRUE(service1->platforms()->Get(1)->arr_platforms()->size() == 1);
  ASSERT_STREQ(
      service1->platforms()->Get(1)->arr_platforms()->Get(0)->name()->c_str(),
      "9");
  ASSERT_TRUE(service1->platforms()->Get(1)->dep_platforms()->size() == 0);

  auto service2 = schedule->services()->Get(1);
  ASSERT_TRUE(service2->times()->Get(0) == -1);
  ASSERT_TRUE(service2->times()->Get(1) == 1059 + 120);
  ASSERT_TRUE(service2->times()->Get(2) == 1388 + 120);
  ASSERT_TRUE(service2->times()->Get(3) == -1);
  ASSERT_TRUE(service2->sections()->size() == 1);
  ASSERT_TRUE(
      !deserialize_bitset<512>(service2->traffic_days()->c_str()).any());
  ASSERT_TRUE(service2->sections()->Get(0)->category()->str() == "ICE");

  auto service3 = schedule->services()->Get(2);
  ASSERT_TRUE(service3->times()->Get(0) == -1);
  ASSERT_TRUE(service3->times()->Get(1) == 1059 + 240);
  ASSERT_TRUE(service3->times()->Get(2) == 1388 + 240);
  ASSERT_TRUE(service3->times()->Get(3) == -1);
  ASSERT_TRUE(service3->sections()->size() == 1);
  ASSERT_TRUE(
      !deserialize_bitset<512>(service3->traffic_days()->c_str()).any());
  ASSERT_TRUE(service3->sections()->Get(0)->category()->str() == "ICE");
}

TEST(loader_hrd_hrd_services, parse_full_schedule) {
  hrd_parser parser;

  const auto schedule_path = SCHEDULES / "multiple-ice-files";
  ASSERT_TRUE(parser.applicable(schedule_path));

  FlatBufferBuilder b;
  parser.parse(schedule_path, b);

  auto schedule = GetSchedule(b.GetBufferPointer());
}

}  // hrd
}  // loader
}  // motis
