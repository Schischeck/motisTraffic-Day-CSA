#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_hotels {

constexpr auto PATH = "modules/reliability/resources/schedule_hotels/";
constexpr auto DATE = "20151019";

schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
schedule_station const LANGEN = {"Langen", "2222222"};
schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
}  // namespace schedule_hotels
}  // namespace reliability
}  // namespace motis
