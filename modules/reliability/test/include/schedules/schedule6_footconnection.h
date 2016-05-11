#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule6 {

constexpr auto PATH = "modules/reliability/resources/schedule6_footconnection/";
constexpr auto DATE = "20151019";

constexpr auto MANNHEIM = schedule_station{"Mannheim", "2222222"};
constexpr auto DARMSTADT = schedule_station{"Darmstadt", "3333333"};
constexpr auto TUD = schedule_station{"TUD", "4444444"};
constexpr auto FRANKFURT = schedule_station{"Frankfurt", "5555555"};
constexpr auto HAUPTWACHE = schedule_station{"Hauptwache", "6666666"};
constexpr unsigned IC_M_D = 1;  // 08:10 --> 08:40
constexpr unsigned RE_T_F = 2;  // 08:45 --> 09:15

}  // namespace schedule6
}  // namespace reliability
}  // namespace motis
