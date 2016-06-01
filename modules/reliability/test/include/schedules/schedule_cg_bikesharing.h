#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_cg_bikesharing {

constexpr auto PATH = "modules/reliability/resources/schedule_cg_bikesharing/";
constexpr auto DATE = "20160204";

constexpr auto FRANKFURT = schedule_station{"Frankfurt", "1111111"};
constexpr auto LANGEN = schedule_station{"Langen", "2222222"};
constexpr auto DARMSTADT = schedule_station{"Darmstadt", "3333333"};
constexpr unsigned RE_D_L = 1;  // 06:00 --> 06:10 (GMT)
constexpr unsigned RE_L_F = 2;  // 06:15 --> 06:25 (GMT)
constexpr unsigned S_L_F = 3;  // 06:16 --> 06:34 (GMT)
constexpr unsigned IC_L_F = 4;  // 06:17 --> 06:40 (GMT)

}  // namespace schedule_cg_bikesharing
}  // namespace reliability
}  // namespace motis
