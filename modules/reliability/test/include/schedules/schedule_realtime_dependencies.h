#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_realtime_dependencies {

constexpr auto PATH =
    "modules/reliability/resources/schedule_realtime_dependencies/";
constexpr auto DATE = "20151019";

constexpr auto DARMSTADT = "3333333";
constexpr auto FRANKFURT = "1111111";
constexpr auto HANAU = "9646170";
constexpr auto LANGEN = "2222222";

/* 08:00 --> 08:10, 08:12 --> 08:20
 * 08:20 --> 08:30, 80:32 --> 08:40
 * 08:40 --> 08:50, 08:52 --> 09:00 */
constexpr unsigned ICE_D_L_F = 1;
/* 08:20 --> 08:35
 * 08:50 --> 09:05
 * 09:20 --> 09:35 */
constexpr unsigned ICE_L_H = 2;
/* 09:10 --> 09:20
 * 10:10 --> 10:20 */
constexpr unsigned ICE_F_H = 3;
/* 09:15 --> 09:25 */
constexpr unsigned RE_H_F = 4;

/* 16:00 --> 16:20 */
constexpr unsigned ICE_D_F = 5;
/* 16:30 --> 16:39 */
constexpr unsigned ICE_L_H_16 = 6;

}  // namespace schedule_realtime_dependencies
}  // namespace reliability
}  // namespace motis
