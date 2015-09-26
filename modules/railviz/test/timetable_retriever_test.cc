#include "catch/catch.hpp"

#include "motis/railviz/timetable_retriever.h"
#include "motis/loader/loader.h"

using namespace motis::railviz;

/**
 * The Tests operate on ../test_timetables/timetable_retriever_test/PLACEHOLDER/motis (relative to
 *build path)
 */

/**
 * Statement Coverage Tests of
 * std::vector<motis::station_node const*> timetable_retriever::stations_on_route( const motis::node& node ) const
 */

//TODO

/**
 * Statement Coverage Tests of
 * std::vector<route> timetable_retriever::get_routes_on_time
 */

//TODO

/**
 * Statement Coverage Tests of
 * timetable timetable_retriever::ordered_timetable_for_station
 */

//TODO

/**
 * Statement Coverage Tests of
 * void timetable_for_station_outgoing
 */

//TODO

/**
 * Statement Coverage Tests of
 * void timetable_for_station_incoming
 */

//TODO

/**
 * Statement Coverage Tests of
 * const motis::node* parent_node(const node &node) const
 */

//TODO
TEST_CASE("01:normal case with an incoming edge", "[railviz::timetable_retriever]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/01_test_set/motis");
    timetable_retriever timetable_retriever_instanz;
    timetable_retriever_instanz.init(*(schedule.get()));
}

/**
 * Statement Coverage Tests of
 * const motis::node* timetable_retriever::child_node(const node &node) const
 */

//TODO

/**
 * Statement Coverage Tests of
 * const motis::node* timetable_retriever::start_node_for_route( const motis::node& node_ ) const
 */

//TODO

/**
 * Statement Coverage Tests of
 * const motis::node* timetable_retriever::end_node_for_route
 */

//TODO

/**
 * Statement Coverage Tests of
 * std::vector<motis::time> timetable_retriever::get_route_departure_times
 */

//TODO
