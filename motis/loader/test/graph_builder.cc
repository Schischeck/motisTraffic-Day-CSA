#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"
#include "motis/loader/parsers/hrd/hrd_parser.h"
#include "motis/loader/graph_builder.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "./hrd/test_spec.h"

namespace motis {
namespace loader {

TEST(loader_graph_builder, simple_test) {
  hrd::hrd_parser parser;

  const auto schedule_path = hrd::SCHEDULES / "multiple-ice-files";
  ASSERT_TRUE(parser.applicable(schedule_path));

  flatbuffers::FlatBufferBuilder b;
  parser.parse(schedule_path, b);
  auto serialized = GetSchedule(b.GetBufferPointer());

  std::time_t interval_begin = to_unix_time(2015, 9, 1);
  std::time_t enterval_end = to_unix_time(2015, 9, 2) + MINUTES_A_DAY * 60;
  auto schedule = build_graph(serialized, interval_begin, enterval_end);
  ASSERT_EQ(interval_begin, schedule->schedule_begin_);
  ASSERT_EQ(enterval_end, schedule->schedule_end_);

  auto& stations = schedule->eva_to_station;
  ASSERT_STREQ("Augsburg Hbf", stations["8000013"]->name.c_str());
  ASSERT_STREQ("Bamberg", stations["8000025"]->name.c_str());
  ASSERT_STREQ("Donauwörth", stations["8000078"]->name.c_str());
  ASSERT_STREQ("Treuchtlingen", stations["8000122"]->name.c_str());
  ASSERT_STREQ("Lichtenfels", stations["8000228"]->name.c_str());
  ASSERT_STREQ("Würzburg Hbf", stations["8000260"]->name.c_str());
  ASSERT_STREQ("München Hbf", stations["8000261"]->name.c_str());
  ASSERT_STREQ("Nürnberg Hbf", stations["8000284"]->name.c_str());
  ASSERT_STREQ("Erlangen", stations["8001844"]->name.c_str());
  ASSERT_STREQ("München-Pasing", stations["8004158"]->name.c_str());
  ASSERT_STREQ("Erfurt Hbf", stations["8010101"]->name.c_str());
  ASSERT_STREQ("Leipzig Hbf", stations["8010205"]->name.c_str());
  ASSERT_STREQ("Lutherstadt Wittenberg", stations["8010222"]->name.c_str());
  ASSERT_STREQ("Naumburg(Saale)Hbf", stations["8010240"]->name.c_str());
  ASSERT_STREQ("Saalfeld(Saale)", stations["8010309"]->name.c_str());
  ASSERT_STREQ("Berlin Gesundbrunnen", stations["8011102"]->name.c_str());
  ASSERT_STREQ("Berlin Südkreuz", stations["8011113"]->name.c_str());
  ASSERT_STREQ("Jena Paradies", stations["8011956"]->name.c_str());
  ASSERT_STREQ("Berlin Hbf (tief)", stations["8098160"]->name.c_str());
}

}  // loader
}  // motis
