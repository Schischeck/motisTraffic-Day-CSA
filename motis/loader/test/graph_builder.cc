#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"
#include "motis/loader/parsers/hrd/hrd_parser.h"
#include "motis/loader/graph_builder.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "./hrd/test_spec.h"

namespace motis {
namespace loader {

auto interval_begin = to_unix_time(2015, 9, 1);
auto interval_end = to_unix_time(2015, 9, 2) + MINUTES_A_DAY * 60;

class graph_builder_test : public ::testing::Test {
protected:
  virtual void SetUp() {
    hrd::hrd_parser parser;

    const auto schedule_path = hrd::SCHEDULES / "multiple-ice-files";
    ASSERT_TRUE(parser.applicable(schedule_path));

    flatbuffers::FlatBufferBuilder b;
    parser.parse(schedule_path, b);
    auto serialized = GetSchedule(b.GetBufferPointer());

    sched_ = build_graph(serialized, interval_begin, interval_end);
  }

  schedule_ptr sched_;
};

TEST_F(graph_builder_test, interval_test) {
  ASSERT_EQ(interval_begin, sched_->schedule_begin_);
  ASSERT_EQ(interval_end, sched_->schedule_end_);
}

TEST_F(graph_builder_test, eva_num) {
  auto& stations = sched_->eva_to_station;
  EXPECT_STREQ("8000013", stations["8000013"]->eva_nr.c_str());
  EXPECT_STREQ("8000025", stations["8000025"]->eva_nr.c_str());
  EXPECT_STREQ("8000078", stations["8000078"]->eva_nr.c_str());
  EXPECT_STREQ("8000122", stations["8000122"]->eva_nr.c_str());
  EXPECT_STREQ("8000228", stations["8000228"]->eva_nr.c_str());
  EXPECT_STREQ("8000260", stations["8000260"]->eva_nr.c_str());
  EXPECT_STREQ("8000261", stations["8000261"]->eva_nr.c_str());
  EXPECT_STREQ("8000284", stations["8000284"]->eva_nr.c_str());
  EXPECT_STREQ("8001844", stations["8001844"]->eva_nr.c_str());
  EXPECT_STREQ("8004158", stations["8004158"]->eva_nr.c_str());
  EXPECT_STREQ("8010101", stations["8010101"]->eva_nr.c_str());
  EXPECT_STREQ("8010205", stations["8010205"]->eva_nr.c_str());
  EXPECT_STREQ("8010222", stations["8010222"]->eva_nr.c_str());
  EXPECT_STREQ("8010240", stations["8010240"]->eva_nr.c_str());
  EXPECT_STREQ("8010309", stations["8010309"]->eva_nr.c_str());
  EXPECT_STREQ("8011102", stations["8011102"]->eva_nr.c_str());
  EXPECT_STREQ("8011113", stations["8011113"]->eva_nr.c_str());
  EXPECT_STREQ("8011956", stations["8011956"]->eva_nr.c_str());
  EXPECT_STREQ("8098160", stations["8098160"]->eva_nr.c_str());
}

TEST_F(graph_builder_test, simple_test) {
  auto& stations = sched_->eva_to_station;
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

TEST_F(graph_builder_test, coordinates) {
  auto& stations = sched_->eva_to_station;

  ASSERT_FLOAT_EQ(48.3654410, stations["8000013"]->width);
  ASSERT_FLOAT_EQ(49.9007590, stations["8000025"]->width);
  ASSERT_FLOAT_EQ(48.7140260, stations["8000078"]->width);
  ASSERT_FLOAT_EQ(48.9612670, stations["8000122"]->width);
  ASSERT_FLOAT_EQ(50.1464520, stations["8000228"]->width);
  ASSERT_FLOAT_EQ(49.8017960, stations["8000260"]->width);
  ASSERT_FLOAT_EQ(48.1402320, stations["8000261"]->width);
  ASSERT_FLOAT_EQ(49.4456160, stations["8000284"]->width);
  ASSERT_FLOAT_EQ(49.5958950, stations["8001844"]->width);
  ASSERT_FLOAT_EQ(48.1498960, stations["8004158"]->width);
  ASSERT_FLOAT_EQ(50.9725510, stations["8010101"]->width);
  ASSERT_FLOAT_EQ(51.3465490, stations["8010205"]->width);
  ASSERT_FLOAT_EQ(51.8675310, stations["8010222"]->width);
  ASSERT_FLOAT_EQ(51.1630710, stations["8010240"]->width);
  ASSERT_FLOAT_EQ(50.6503160, stations["8010309"]->width);
  ASSERT_FLOAT_EQ(52.5489630, stations["8011102"]->width);
  ASSERT_FLOAT_EQ(52.4750470, stations["8011113"]->width);
  ASSERT_FLOAT_EQ(50.9248560, stations["8011956"]->width);
  ASSERT_FLOAT_EQ(52.5255920, stations["8098160"]->width);

  ASSERT_FLOAT_EQ(10.8855700, stations["8000013"]->length);
  ASSERT_FLOAT_EQ(10.8994890, stations["8000025"]->length);
  ASSERT_FLOAT_EQ(10.7714430, stations["8000078"]->length);
  ASSERT_FLOAT_EQ(10.9081590, stations["8000122"]->length);
  ASSERT_FLOAT_EQ(11.0594720, stations["8000228"]->length);
  ASSERT_FLOAT_EQ(9.93578000, stations["8000260"]->length);
  ASSERT_FLOAT_EQ(11.5583350, stations["8000261"]->length);
  ASSERT_FLOAT_EQ(11.0829890, stations["8000284"]->length);
  ASSERT_FLOAT_EQ(11.0019080, stations["8001844"]->length);
  ASSERT_FLOAT_EQ(11.4614850, stations["8004158"]->length);
  ASSERT_FLOAT_EQ(11.0384990, stations["8010101"]->length);
  ASSERT_FLOAT_EQ(12.3833360, stations["8010205"]->length);
  ASSERT_FLOAT_EQ(12.6620150, stations["8010222"]->length);
  ASSERT_FLOAT_EQ(11.7969840, stations["8010240"]->length);
  ASSERT_FLOAT_EQ(11.3749870, stations["8010309"]->length);
  ASSERT_FLOAT_EQ(13.3885130, stations["8011102"]->length);
  ASSERT_FLOAT_EQ(13.3653190, stations["8011113"]->length);
  ASSERT_FLOAT_EQ(11.5874610, stations["8011956"]->length);
  ASSERT_FLOAT_EQ(13.3695450, stations["8098160"]->length);
}

}  // loader
}  // motis
