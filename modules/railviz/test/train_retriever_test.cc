#include "gtest/gtest.h"
#include <vector>
#include "motis/core/common/date_util.h"
#include "motis/railviz/train_retriever.h"
#include "motis/loader/loader.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/synced_schedule.h"
#include "motis/railviz/geo.h"
using namespace motis::railviz;
using namespace motis;

TEST(railviz_all_train, simple_test){
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));																	
  train_retriever ttr(*schedule);
	geo::box area = {{49.8728, 8.24411},
                   {50.969, 9.79821}};
	std::vector<std::pair<light_connection const*, edge const*>> trains = ttr.trains(2800, 3100, 1000, area);
	ASSERT_NE(0, trains.size());
	ASSERT_EQ(5, trains.size());
}