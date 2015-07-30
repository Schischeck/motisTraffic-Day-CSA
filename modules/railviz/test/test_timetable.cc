#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/railviz.h"
#include "motis/railviz/train_retriever.h"
#include "json11/json11.hpp"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test_timetable {

struct Fixure
{
  Fixure()
  {
    schedule = motis::load_schedule("../test_timetables/date_manager_test1/motis");
    std::cout << "schedule loaded" << std::endl;
    r.schedule_ = schedule.get();
    r.init();
  }

  motis::schedule_ptr schedule;
  railviz r;
};

BOOST_FIXTURE_TEST_SUITE( railviz_test_timetable, Fixure )

BOOST_AUTO_TEST_CASE( railviz_timetable_test )
{
  std::cout << "start request" << std::endl;
  int station_id = 2;
  std::vector<train> trains = r.train_retriever_.get()->timetable_for_station_outgoing(*schedule.get()->station_nodes[station_id].get());

  std::cout << "departure from " << schedule.get()->stations[station_id].get()->name << std::endl;
  for(train &t : trains)
  {
    std::cout << t.light_conenction_->d_time;
    std::cout << " Gleis: " << t.light_conenction_->_full_con->d_platform;
    std::cout << " Klasse:" << (int)t.light_conenction_->_full_con->clasz;
    std::cout << " Ziel: " << schedule.get()->stations[t.a_station].get()->name;
    std::cout << " Gleis: " << t.light_conenction_->_full_con->a_platform << std::endl;
  }
  std::cout << "end request" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
