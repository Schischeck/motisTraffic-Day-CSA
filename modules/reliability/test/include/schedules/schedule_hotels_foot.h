#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_hotels_foot {

constexpr auto PATH = "modules/reliability/resources/schedule_hotels_foot/";
constexpr auto HOTELS =
    "modules/reliability/resources/schedule_hotels_foot/hotels.csv";
constexpr auto DATE = "20151019";

schedule_station const FRANKFURT = {"Frankfurt", "8011111"};
schedule_station const LANGEN = {"Langen", "8022222"};
schedule_station const DARMSTADT = {"Darmstadt", "8033333"};
schedule_station const NEUISENBURG = {"Neu-Isenburg", "8044444"};
schedule_station const BERLIN = {"Berlin", "8055555"};

}  // namespace schedule_hotels_foot
}  // namespace reliability
}  // namespace motis
