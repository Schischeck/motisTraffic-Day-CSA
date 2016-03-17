#include "gtest/gtest.h"

#include "motis/test/motis_instance_helper.h"

using namespace motis::test;

namespace motis {
namespace rt {

TEST(rt, itest) {

  auto instance = launch_motis("modules/rt/test_resources/magic", "20160128",
                               {"ris", "rt"},{
                                "--ris.zip_folder=modules/rt/test_resources/magic/ris",
                                "--ris.sim_init_start=1454108400",
                                "--ris.sim_init_end=1454108400",
                               });

}

}  // namespace rt
}  // namespace motis