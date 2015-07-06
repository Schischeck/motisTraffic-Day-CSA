#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/date_converter.h"
#include "motis/railviz/train_retriever.h"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test_train_query {

struct Fixure
{
    Fixure()
    {
        schedule = motis::load_schedule("../test_timetables/train_query_test/motis");

        train_query_ptr = std::unique_ptr<motis::railviz::train_retriever>(new train_retriever(*schedule));
    }

    std::unique_ptr<motis::railviz::train_retriever> train_query_ptr;
    motis::schedule_ptr schedule;
};

BOOST_FIXTURE_TEST_SUITE( railviz_train_query, Fixure )

BOOST_AUTO_TEST_CASE( railviz_train_query_time_interval )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_retriever& train_query = *train_query_ptr.get();

    geo::coord left_top{ 50, 10 };
    geo::coord right_bottom{ 51, 11 };
    geo::box b{left_top, right_bottom};
    motis::date_manager::date date = schedule.get()->date_mgr.first_date();
    time from = date_converter.convert_to_motis( date );
    date.day = date.day+1;
    time to = date_converter.convert_to_motis( date );

    train_list_ptr trainlist = train_query.trains(from, to, b);
    BOOST_CHECK( trainlist.get()->size() == 10 );

    date.day = date.day + 1;
    to = date_converter.convert_to_motis( date );
    trainlist = train_query.trains(from, to, b);
    BOOST_CHECK( trainlist.get()->size() == 20 );
}

BOOST_AUTO_TEST_CASE( railviz_train_query_limit )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_retriever& train_query = *train_query_ptr.get();

    geo::coord left_top{ 50, 10 };
    geo::coord right_bottom{ 51, 11 };
    geo::box b{left_top, right_bottom};
    motis::date_manager::date date = schedule.get()->date_mgr.first_date();
    time from = date_converter.convert_to_motis( date );
    date = schedule.get()->date_mgr.last_date();
    time to = date_converter.convert_to_motis( date );

    train_list_ptr trainlist = train_query.trains(from, to, b, 42);
    BOOST_CHECK( trainlist.get()->size() == 42 );

    trainlist = train_query.trains(from, to, b, 142);
    BOOST_CHECK( trainlist.get()->size() == 142 );

    trainlist = train_query.trains(from, to, b, 259);
    BOOST_CHECK( trainlist.get()->size() == 200 );
}

BOOST_AUTO_TEST_CASE( railviz_train_query_bounds )
{
    motis::railviz::date_converter date_converter( schedule.get()->date_mgr );
    motis::railviz::train_retriever& train_query = *train_query_ptr.get();

    geo::coord left_top{ 50, 10 };
    geo::coord right_bottom{ 51, 11 };
    geo::box b{left_top, right_bottom};
    motis::date_manager::date date = schedule.get()->date_mgr.first_date();
    time from = date_converter.convert_to_motis( date );
    date.day = date.day+1;
    time to = date_converter.convert_to_motis( date );

    train_list_ptr trainlist = train_query.trains(from, to, b);
    BOOST_CHECK( trainlist.get()->size() == 10 );

    left_top = geo::coord{50.620, 10.583};
    right_bottom = geo::coord{50.640, 10.598};
    b = geo::box{left_top, right_bottom};

    trainlist = train_query.trains(from, to, b);
    BOOST_CHECK( trainlist.get()->size() == 5 );

    left_top = geo::coord{50.630, 10.753};
    right_bottom = geo::coord{50.652, 10.785};
    b = geo::box{left_top, right_bottom};

    trainlist = train_query.trains(from, to, b);
    BOOST_CHECK( trainlist.get()->size() == 2 );
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
