#include "gtest/gtest.h"

#include "motis/test/motis_instance_helper.h"

using namespace motis::test;

namespace motis {
namespace rt {

TEST(rt, rename_at_first_stop) {
  auto instance =
      launch_motis("modules/rt/test_resources/rename_at_first_stop", "20160128",
                   {"ris", "rt"}, {"--ris.mode=test",
                                   "--ris.input_folder=modules/rt/"
                                   "test_resources/rename_at_first_stop/ris"});
}

}  // namespace rt
}  // namespace motis