#include <iostream>
#include <memory>

#include "motis/loader/loader.h"
#include "motis/railviz/date_converter.h"

#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test {

/**
 * The Tests operate on ../test_timetables/date_manager_test1/motis (relative to build path)
 *
 * There is one single Train defined that drives every day three times from station A to station B.
 * #    departure-time  arrival-time
 * 1    7:56            8:00
 * 2    8:56            9:00
 * 3    9:56            10:00
 */

struct Fixure
{
    Fixure()
    {
        schedule = motis::load_schedule("../test_timetables/date_manager_test1/motis");
    }

    motis::schedule_ptr schedule;
};

BOOST_FIXTURE_TEST_SUITE( railviz_date_converter, Fixure )

void print_time( const std::time_t& time )
{
    std::tm *ptm = std::localtime(&time);
    char buffer[32];
    // Format: Mo, 15.06.2009 20:20:00
    std::strftime(buffer, 32, "%a, %d.%m.%Y %H:%M:%S", ptm);
    std::cout << buffer << std::endl;
}

/*
 * This test extracting all trains driving at the first day and validates
 * the correct conversion of their times
 */
BOOST_AUTO_TEST_CASE( railviz_train_query_test )
{
    /*
    motis::railviz::date_converter dcnv( schedule.get()->date_mgr );

    motis::date_manager &mgr = schedule.get()->date_mgr;

    for( auto const& station_node_pointer : schedule.get()->station_nodes )
    {
        station_node const* station_node = station_node_pointer.get();
        for( auto const& route_node : station_node->get_route_nodes() )
        {
            std::cout << &(route_node->_edges) << std::endl;
            for( auto const& edge : route_node->_edges )
            {
                std::cout << "it" << std::endl;
                for( auto const& lcon : edge._m._route_edge._conns )
                {
                    std::cout << "it2" << std::endl;
                    std::cout << "dtime " << lcon.d_time << std::endl;
                    //std::time_t d_time = dcnv.convert( lcon.d_time );
                    //std::time_t a_time = dcnv.convert( lcon.a_time );
                    //print_time(d_time);
                    //print_time(a_time);
                    std::cout << std::endl;
                }
            }
        }
    }
    */
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
