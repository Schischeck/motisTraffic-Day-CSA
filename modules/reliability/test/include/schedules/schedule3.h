#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule3 {

constexpr auto PATH = "modules/reliability/resources/schedule3/";
constexpr auto DATE = "20150928";

constexpr auto FRANKFURT = schedule_station{"Frankfurt", "1111111"};
constexpr auto MESSE = schedule_station{"Frankfurt Messe", "2222222"};
constexpr auto LANGEN = schedule_station{"Langen", "3333333"};
constexpr auto WEST = schedule_station{"Frankfurt West", "4444444"};

constexpr unsigned ICE_L_H = 1;  // 10:00 --> 10:10
constexpr unsigned S_M_W = 2;  // 10:20 --> 10:25

}  // namespace schedule3
}  // namespace reliability
}  // namespace motis
