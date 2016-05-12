#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule2 {

constexpr auto PATH = "modules/reliability/resources/schedule2/";
constexpr auto DATE = "20150928";

constexpr auto ERLANGEN = schedule_station{"Erlangen", "0953067"};
constexpr auto FRANKFURT = schedule_station{"Frankfurt", "5744986"};
constexpr auto KASSEL = schedule_station{"Kassel", "6380201"};
constexpr auto STUTTGART = schedule_station{"Stuttgart", "7309882"};
constexpr auto HEILBRONN = schedule_station{"Heilbronn", "1584227"};

constexpr unsigned RE_K_F = 1;  // 08:00 --> 10:00
constexpr unsigned ICE_F_S = 2;  // 10:10 --> 11:10
constexpr unsigned ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
constexpr unsigned S_N_E = 4;  // 11:30 --> 15:30
constexpr unsigned ICE_S_E = 5;  // 11:32 --> 12:32
constexpr unsigned S_H_S = 6;  // 07:15 --> 11:15
constexpr unsigned ICE_E_K = 7;  // 12:45 --> 14:15

}  // namespace schedule2
}  // namespace reliability
}  // namespace motis
