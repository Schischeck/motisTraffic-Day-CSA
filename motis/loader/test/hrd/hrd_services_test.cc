#include <cinttypes>
#include <string>
#include <iostream>

#include "gtest/gtest.h"

#include "parser/cstr.h"

#include "motis/loader/hrd/model/hrd_service.h"

#include "./test_spec_test.h"

using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_hrd_services, simple_ranges) {
  test_spec services_file(SCHEDULES / "hand-crafted" / "fahrten",
                          "services-1.101");

  auto services = services_file.get_hrd_services();
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
        EXPECT_EQ(std::vector<cstr>({"381"}), s.line_information);
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
  ASSERT_FALSE(stop.arr.in_out_allowed);
  ASSERT_TRUE(stop.dep.time == hrd_service::NOT_SET);
}

TEST(loader_hrd_hrd_services, complex_ranges) {
  test_spec services_file(SCHEDULES / "hand-crafted" / "fahrten",
                          "services-2.101");

  auto services = services_file.get_hrd_services();
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
  ASSERT_TRUE(section.line_information == std::vector<cstr>({"381"}));
  ASSERT_TRUE(section.traffic_days == std::vector<int>({0}));

  section = service.sections_[1];
  ASSERT_TRUE(section.train_num == 2293);
  ASSERT_TRUE(section.admin == "81____");
  ASSERT_TRUE(section.attributes == std::vector<hrd_service::attribute>(
                                        {hrd_service::attribute(1337, "BT")}));
  ASSERT_TRUE(section.category == std::vector<cstr>({"IC "}));
  ASSERT_TRUE(section.line_information == std::vector<cstr>({"381"}));
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

TEST(loader_hrd_hrd_services, indices) {
  test_spec services_file(SCHEDULES / "single-index-bus" / "fahrten",
                          "services.101");

  auto services = services_file.get_hrd_services();
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

TEST(loader_hrd_hrd_services, time_prefixes) {
  test_spec services_file(SCHEDULES / "complex-ranges" / "fahrten",
                          "services.101");

  auto services = services_file.get_hrd_services();
  ASSERT_TRUE(services.size() == 1);

  auto const& service = services[0];
  ASSERT_TRUE(service.sections_.size() == 16);

  std::for_each(
      std::begin(service.sections_), std::end(service.sections_),
      [](hrd_service::section const& section) {
        ASSERT_TRUE(section.train_num == 0);
        ASSERT_TRUE(section.admin == "rmv106");
        ASSERT_TRUE(section.category == std::vector<cstr>({"rfb"}));
        ASSERT_TRUE(section.line_information == std::vector<cstr>({"56"}));
        ASSERT_TRUE(section.traffic_days == std::vector<int>({32283}));
        ASSERT_TRUE(section.attributes == std::vector<hrd_service::attribute>(
                                              {{0, "g5"}, {0, "j4"}}));
      });
}

}  // hrd
}  // loader
}  // motis
