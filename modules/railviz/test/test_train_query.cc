#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/edge_geo_index.h"
#include "motis/railviz/train_query.h"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test {
/*
struct Fixure
{
    Fixure()
    {
        schedule = motis::load_schedule("../test_timetables/trutzhain/motis");

        geo_index = std::unique_ptr<motis::railviz::edge_geo_index>(new edge_geo_index(*schedule));
    }

    std::unique_ptr<motis::railviz::edge_geo_index> geo_index;
    motis::schedule_ptr schedule;
};

BOOST_FIXTURE_TEST_SUITE( railviz_train_query, Fixure )

BOOST_AUTO_TEST_CASE( railviz_train_query_test )
{
    motis::railviz::date_converter dcnv( schedule.get()->date_mgr );
    motis::railviz::train_query tq( *geo_index, dcnv );

    std::cout << "eveything initialized fine" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
*/
}
}
}
