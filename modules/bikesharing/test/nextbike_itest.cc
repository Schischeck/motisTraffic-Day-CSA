#include "gtest/gtest.h"

#include <iostream>

#include "motis/test/motis_instance_helper.h"
#include "motis/module/message.h"

using namespace motis::test;
using namespace motis::module;

namespace motis {
namespace bikesharing {

constexpr auto kBikesharingRequest = R""(
{
  "content_type": "BikesharingRequest",
  "content": {
    "departure_lat": 49.8776114,
    "departure_lng": 8.6571044,
    
    "arrival_lat": 49.8725312,
    "arrival_lng": 8.6295316,

    "window_begin": 1454602500,
    "window_end": 1454606100
  }
}
)"";

TEST(bikesharing_nextbike_itest, integration_test) {
  auto instance = launch_motis("modules/bikesharing/test_resources/schedule",
                               "20150112", {"bikesharing", "intermodal"},
                               {"--bikesharing.nextbike_path=modules/"
                                "bikesharing/test_resources/nextbike",
                                "--bikesharing.database_path=:memory:"});


  auto resp = send(instance, make_msg(kBikesharingRequest));
  std::cout << resp << std::endl;
}

}  // namespace bikesharing
}  // namespace motis