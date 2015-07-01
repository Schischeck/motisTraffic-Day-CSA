#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/edge_geo_index.h"
#include "motis/railviz/train_query.h"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test_train_query {

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

BOOST_AUTO_TEST_CASE( railviz_edge_geo_index_test )
{
    motis::railviz::edge_geo_index index( * schedule.get() );
    geometry::point left_top( 50, 10 );
    geometry::point right_bottom( 51, 11 );
    geometry::box b( left_top, right_bottom );

    std::vector<const motis::edge*> edges = index.edges(right_bottom.lat, right_bottom.lng, left_top.lat, left_top.lng);
    BOOST_CHECK(edges.size() == 10);
}

BOOST_AUTO_TEST_CASE( railviz_train_query_time_interval )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_query train_query( *geo_index, date_converter );

    geometry::point left_top( 50, 10 );
    geometry::point right_bottom( 51, 11 );
    geometry::box b(left_top, right_bottom);
    motis::date_manager::date date = schedule.get()->date_mgr.first_date();
    std::time_t from = date_converter.convert( date );
    date.day = date.day+1;
    std::time_t to = date_converter.convert( date );

    train_list_ptr trainlist = train_query.by_bounds_and_time_interval(b, from, to);
    BOOST_CHECK( trainlist.get()->size() == 10 );

    date.day = date.day + 1;
    to = date_converter.convert( date );
    trainlist = train_query.by_bounds_and_time_interval(b, from, to);
    BOOST_CHECK( trainlist.get()->size() == 20 );
}

BOOST_AUTO_TEST_CASE( railviz_train_query_limit )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_query train_query( *geo_index, date_converter );

    geometry::point left_top( 50, 10 );
    geometry::point right_bottom( 51, 11 );
    geometry::box b(left_top, right_bottom);
    motis::date_manager::date date = schedule.get()->date_mgr.first_date();
    std::time_t from = date_converter.convert( date );
    date = schedule.get()->date_mgr.last_date();
    std::time_t to = date_converter.convert( date );

    train_list_ptr trainlist = train_query.by_bounds_and_time_interval(b, from, to, 42);
    BOOST_CHECK( trainlist.get()->size() == 42 );

    trainlist = train_query.by_bounds_and_time_interval(b, from, to, 142);
    BOOST_CHECK( trainlist.get()->size() == 142 );

    trainlist = train_query.by_bounds_and_time_interval(b, from, to, 259);
    BOOST_CHECK( trainlist.get()->size() == 200 );
}

BOOST_AUTO_TEST_CASE( railviz_train_query_bounds )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_query train_query( *geo_index, date_converter );

    geometry::point left_top( 50, 10 );
    geometry::point right_bottom( 51, 11 );
    geometry::box b(left_top, right_bottom);
    motis::date_manager::date date = schedule.get()->date_mgr.first_date();
    std::time_t from = date_converter.convert( date );
    date.day = date.day+1;
    std::time_t to = date_converter.convert( date );

    train_list_ptr trainlist = train_query.by_bounds_and_time_interval(b, from, to);
    BOOST_CHECK( trainlist.get()->size() == 10 );

    left_top = geometry::point(50.620, 10.583);
    right_bottom = geometry::point(50.640, 10.598);
    b = geometry::box(left_top, right_bottom);

    trainlist = train_query.by_bounds_and_time_interval(b, from, to);
    BOOST_CHECK( trainlist.get()->size() == 5 );

    left_top = geometry::point(50.630, 10.753);
    right_bottom = geometry::point(50.652, 10.785);
    b = geometry::box(left_top, right_bottom);

    trainlist = train_query.by_bounds_and_time_interval(b, from, to);
    BOOST_CHECK( trainlist.get()->size() == 2 );
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
