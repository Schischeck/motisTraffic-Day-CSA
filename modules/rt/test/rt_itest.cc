#include "gtest/gtest.h"

#include <algorithm>
#include <memory>

#include "motis/test/motis_instance_helper.h"
#include "motis/module/module.h"

#include "motis/test/schedule/rename_at_first_stop.h"

using namespace motis::test;
using namespace motis::test::schedule::rename_at_first_stop;
using namespace motis::module;

namespace motis {
namespace rt {

TEST(rt, rename_at_first_stop) {
  auto instance = launch_motis(kSchedulePath, kScheduleDate, {"ris", "rt"},
                               {"--ris.mode=test", kRisFolderArg});

  auto it = std::find_if(begin(instance->modules_), end(instance->modules_),
                         [](std::unique_ptr<motis::module::module> const& m) {
                           return m->name() == "rt";
                         });
  ASSERT_NE(it, end(instance->modules_));
  auto stats = static_cast<rt*>(it->get())->last_run_stats_;

  EXPECT_EQ(1, stats.delay.trips_.found);
  EXPECT_EQ(2, stats.assessment.trips_.found);
}

}  // namespace rt
}  // namespace motis