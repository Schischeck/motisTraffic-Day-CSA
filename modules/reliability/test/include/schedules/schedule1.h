#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule1 {

constexpr auto PATH = "modules/reliability/resources/schedule/";
constexpr auto DATE = "20150928";

constexpr auto DARMSTADT = "4219971";
constexpr auto FRANKFURT = "8351230";
constexpr auto KARLSRUHE = "7226036";
constexpr auto WURZBURG = "0064254";
constexpr auto HEIDELBERG = "9335048";
/* train numbers */
constexpr unsigned IC_DA_H = 1;
constexpr unsigned IC_FR_DA = 2;
constexpr unsigned IC_FH_DA = 3;
constexpr unsigned RE_MA_DA = 4;
constexpr unsigned ICE_FR_DA_H = 5;
constexpr unsigned ICE_HA_W_HE = 6;
constexpr unsigned ICE_K_K = 7;
constexpr unsigned RE_K_S = 8;

}  // namespace schedule1
}  // namespace reliability
}  // namespace motis
