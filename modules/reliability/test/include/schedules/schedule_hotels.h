#pragma once

#include "../test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace schedule_hotels {

constexpr auto PATH = "test/schedule/schedule_hotels/";
constexpr auto PATH_FOOT_SCHEDULE =
    "modules/reliability/resources/schedule_hotels_foot/";
constexpr auto DATE = "20151019";

schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
schedule_station const LANGEN = {"Langen", "2222222"};
schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
schedule_station const MAINZ = {"Mainz", "3953754"};
schedule_station const NEUISENBURG = {"Neu-Isenburg", "5345291"};
schedule_station const OFFENBACH = {"Offenbach", "9727248"};
schedule_station const WALLDORF = {"Walldorf", "2813399"};

}  // namespace schedule_hotels
}  // namespace reliability
}  // namespace motis
