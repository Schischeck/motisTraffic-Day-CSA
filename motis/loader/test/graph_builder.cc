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

  std::time_t begin = to_unix_time(2015, 9, 1);
  std::time_t end = to_unix_time(2015, 9, 2) + MINUTES_A_DAY * 60;
  auto schedule = build_graph(serialized, begin, end);
  ASSERT_EQ(begin, schedule->schedule_begin_);
  ASSERT_EQ(end, schedule->schedule_end_);
}

}  // loader
}  // motis
