#include "gtest/gtest.h"

#include <algorithm>
#include <memory>

#include "motis/test/motis_instance_helper.h"
#include "motis/module/module.h"

using namespace motis::test;
using namespace motis::module;

namespace motis {
namespace rt {

TEST(rt, rename_at_first_stop) {
  auto instance =
      launch_motis("modules/rt/test_resources/rename_at_first_stop", "20160128",
                   {"ris", "rt"}, {"--ris.mode=test",
                                   "--ris.input_folder=modules/rt/"
                                   "test_resources/rename_at_first_stop/ris"});

  auto it = std::find_if(begin(instance->modules_), end(instance->modules_),
                         [](std::unique_ptr<motis::module::module> const& m) {
                           return m->name() == "rt";
                         });
  ASSERT_NE(it, end(instance->modules_));
  ASSERT_EQ(1, static_cast<rt*>(it->get())->successful_trip_lookups);
}

}  // namespace rt
}  // namespace motis