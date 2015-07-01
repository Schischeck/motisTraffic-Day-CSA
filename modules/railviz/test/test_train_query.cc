#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/edge_geo_index.h"
#include "motis/railviz/train_query.h"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test {

struct Fixure
{
    Fixure()
    {
        schedule = motis::load_schedule("../test_timetables/train_query_test/motis");

        geo_index = std::unique_ptr<motis::railviz::edge_geo_index>(new edge_geo_index(*schedule));
    }

    std::unique_ptr<motis::railviz::edge_geo_index> geo_index;
    motis::schedule_ptr schedule;
};

BOOST_FIXTURE_TEST_SUITE( railviz_train_query, Fixure )

BOOST_AUTO_TEST_CASE( railviz_train_query_all_test )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_query train_query( *geo_index, date_converter );

    geometry::point left_top( 50.62, 10.4 );
    geometry::point right_bottom( 50.65, 10.8 );
    geometry::box b(left_top, right_bottom);
    std::time_t from = date_converter.convert( schedule.get()->date_mgr.first_date() );
    std::time_t to = date_converter.convert( schedule.get()->date_mgr.last_date() );

    train_list_ptr trainlist = train_query.by_bounds_and_time_interval(b, from, to);
    //std::cout << "size: " << trainlist.get()->size() << std::endl;

    std::cout << "eveything initialized fine" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
