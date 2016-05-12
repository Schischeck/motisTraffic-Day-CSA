#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule7_cg {

constexpr auto PATH = "modules/reliability/resources/schedule7_cg/";
constexpr auto DATE = "20151019";

constexpr auto FRANKFURT = schedule_station{"Frankfurt", "1111111"};
constexpr auto LANGEN = schedule_station{"Langen", "2222222"};
constexpr auto DARMSTADT = schedule_station{"Darmstadt", "3333333"};
constexpr auto PFUNGSTADT = schedule_station{"Pfungstadt", "5420132"};
constexpr unsigned RE_D_L = 1;  // 07:00 --> 07:10
constexpr unsigned RE_L_F = 2;  // 07:15 --> 07:25
constexpr unsigned S_L_F = 3;  // 07:16 --> 07:34
constexpr unsigned IC_L_F = 4;  // 07:17 --> 07:40

}  // namespace schedule7_cg
}  // namespace reliability
}  // namespace motis
