#include "gtest/gtest.h"

#include <iostream>

#include "motis/test/motis_instance_helper.h"

using namespace motis::test;

namespace motis {
namespace bikesharing {

constexpr double departure_lat = 49.8776114;
constexpr double departure_lng = 8.6571044;

constexpr double arrival_lat = 49.8725312;
constexpr double arrival_lng = 8.6295316;

constexpr std::time_t window_begin = 1454602500;
constexpr std::time_t window_end = 1454606100;

TEST(bikesharing_nextbike_itest, integration_test) {
  auto instance = launch_motis(
      "module/bikesharing/test_resources/schedule", "20150112",
      {"bikesharing", "intermodal"},
      {"--bikesharing.nextbike_path=module/bikesharing/test_resources/nextbike"});

}

}  // namespace bikesharing
}  // namespace motis