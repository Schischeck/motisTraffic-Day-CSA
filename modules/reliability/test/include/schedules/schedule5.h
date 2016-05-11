#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule5 {

constexpr auto PATH = "modules/reliability/resources/schedule5/";
constexpr auto DATE = "20151019";

constexpr auto DARMSTADT = schedule_station{"Darmstadt", "1111111"};
constexpr auto FRANKFURT = schedule_station{"Frankfurt", "2222222"};
constexpr auto GIESSEN = schedule_station{"Giessen", "3333333"};
constexpr auto MARBURG = schedule_station{"Marburg", "4444444"};
constexpr auto BENSHEIM = schedule_station{"Bensheim", "5555555"};
constexpr auto MANNHEIM = schedule_station{"Mannheim", "6666666"};
constexpr unsigned RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
constexpr unsigned RE_G_M = 2;  // 09:10 --> 09:40
constexpr unsigned RE_M_B_D = 3;  // 07:00 --> 07:30, 07:31 --> 07:55

}  // namespace schedule5
}  // namespace reliability
}  // namespace motis
