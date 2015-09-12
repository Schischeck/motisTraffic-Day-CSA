#include <cinttypes>
#include <string>
#include <iostream>

#include "gtest/gtest.h"

#include "test_spec.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/parsers/hrd/hrd_parser.h"

using namespace parser;
using namespace flatbuffers;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_fbs_services, repeated_service) {
  auto const hrd_root = SCHEDULES / "repeated-service";
  hrd_parser p;
  FlatBufferBuilder b;

  ASSERT_TRUE(p.applicable(hrd_root));

  p.parse(hrd_root, b);
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
  ASSERT_STREQ("ICE",
               service1->sections()->Get(0)->category()->name()->c_str());
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
  ASSERT_TRUE(service2->sections()->Get(0)->category()->name()->str() == "ICE");

  auto service3 = schedule->services()->Get(2);
  ASSERT_TRUE(service3->times()->Get(0) == -1);
  ASSERT_TRUE(service3->times()->Get(1) == 1059 + 240);
  ASSERT_TRUE(service3->times()->Get(2) == 1388 + 240);
  ASSERT_TRUE(service3->times()->Get(3) == -1);
  ASSERT_TRUE(service3->sections()->size() == 1);
  ASSERT_TRUE(
      !deserialize_bitset<512>(service3->traffic_days()->c_str()).any());
  ASSERT_TRUE(service3->sections()->Get(0)->category()->name()->str() == "ICE");
}

TEST(loader_hrd_fbs_services, multiple_services) {
  auto const hrd_root = SCHEDULES / "multiple-ices";
  hrd_parser p;
  FlatBufferBuilder b;

  ASSERT_TRUE(p.applicable(hrd_root));

  p.parse(hrd_root, b);
}

TEST(loader_hrd_fbs_services, multiple_files) {
  auto const hrd_root = SCHEDULES / "multiple-ice-files";
  hrd_parser p;
  FlatBufferBuilder b;

  ASSERT_TRUE(p.applicable(hrd_root));

  p.parse(hrd_root, b);
}

TEST(loader_hrd_fbs_services, direction_service) {
  auto const hrd_root = SCHEDULES / "direction-services";
  hrd_parser p;
  FlatBufferBuilder b;

  ASSERT_TRUE(p.applicable(hrd_root));

  p.parse(hrd_root, b);
  auto schedule = GetSchedule(b.GetBufferPointer());

  ASSERT_EQ(7, schedule->services()->size());

  for (int i = 0; i < 4; i++) {
    for (auto const& section : *schedule->services()->Get(i)->sections()) {
      ASSERT_STREQ("Krofdorf-Gleiberg Evangelische Ki",
                   section->direction()->text()->c_str());
      ASSERT_FALSE(section->direction()->station());
    }
  }

  for (int service_idx = 4; service_idx < 6; service_idx++) {
    auto section_idx = 0;
    for (auto const& section :
         *schedule->services()->Get(service_idx)->sections()) {
      if (section->direction()) {
        ASSERT_TRUE(0 <= section_idx && section_idx <= 11);
        ASSERT_FALSE(section->direction()->text());
        ASSERT_STREQ("8003436", section->direction()->station()->id()->c_str());
      }
      ++section_idx;
    }
  }

  auto const last_service = schedule->services()->Get(6);
  for (auto const& section : *last_service->sections()) {
    ASSERT_FALSE(section->direction()->text());
    ASSERT_STREQ("8000310", section->direction()->station()->id()->c_str());
  }
}

}  // hrd
}  // loader
}  // motis
