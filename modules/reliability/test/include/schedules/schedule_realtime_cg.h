#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_realtime_cg {

constexpr auto PATH = "modules/reliability/resources/schedule_realtime_cg/";
constexpr auto DATE = "20151019";

constexpr static schedule_station FRANKFURT = {"Frankfurt", "1111111"};
constexpr static schedule_station LANGEN = {"Langen", "2222222"};
constexpr static schedule_station DARMSTADT = {"Darmstadt", "3333333"};
constexpr static unsigned RE_D_L = 1;  // 07:00 --> 07:10 (+1 is-delay)
constexpr static unsigned RE_L_F = 2;  // 07:15 (+1 forecast) --> 07:25
constexpr static unsigned IC_L_F = 4;  // 07:17 --> 07:40

}  // namespace schedule_realtime_cg
}  // namespace reliability
}  // namespace motis
