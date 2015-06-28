#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/date_converter.h"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test {

struct Fixure
{
    Fixure()
    {
        schedule = motis::load_schedule("../test_timetables/date_manager_test1/motis");
    }

    motis::schedule_ptr schedule;
};

BOOST_FIXTURE_TEST_SUITE( railviz_date_converter, Fixure )

BOOST_AUTO_TEST_CASE( railviz_train_query_test )
{
    motis::railviz::date_converter dcnv( schedule.get()->date_mgr );

    std::cout << "eveything initialized fine" << std::endl;

    motis::date_manager &mgr = schedule.get()->date_mgr;
    std::cout << mgr.first_date().day << "." << mgr.first_date().month << "." << mgr.first_date().year << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
