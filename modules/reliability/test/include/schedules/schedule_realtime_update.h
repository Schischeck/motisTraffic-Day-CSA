#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_realtime_update {

constexpr auto PATH = "modules/reliability/resources/schedule_realtime_update/";
constexpr auto DATE = "20151019";

constexpr static auto DARMSTADT = "3333333";
constexpr static auto FRANKFURT = "1111111";
constexpr static auto HANAU = "9646170";
constexpr static auto LANGEN = "2222222";

/* 08:00 --> 08:10, 08:13 --> 08:18 */
constexpr static unsigned ICE_D_L_F = 1;
/* 08:45 --> 09:00 */
constexpr static unsigned ICE_F_H = 3;

}  // namespace schedule_realtime_update
}  // namespace reliability
}  // namespace motis
