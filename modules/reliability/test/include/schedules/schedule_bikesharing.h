#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_bikesharing {

constexpr auto PATH = "modules/reliability/resources/schedule_bikesharing/";
constexpr auto DATE = "20150115";

constexpr auto FRANKFURT = schedule_station{"Frankfurt(Main)Hbf", "4570367"};
constexpr auto DARMSTADT = schedule_station{"Darmstadt Hbf", "2158078"};
constexpr unsigned IC_D_F = 1;  // 17:00 --> 17:15 (GMT)
constexpr unsigned IC_D_F_2 = 2;  // 16:35 --> 16:40 (GMT)

}  // namespace schedule_bikesharing
}  // namespace reliability
}  // namespace motis
