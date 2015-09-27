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

TEST_CASE("01:test case without loop", "[railviz::timetable_retriever::parent_node]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/01_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;

    it = schedule->eva_to_station.find(5001307);
    SECTION("eva_to_station conversion of the 01 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_01_routenode;
    motis::station* st_01 = it->second;
    motis::station_node* st_01_stnode = schedule->station_nodes[st_01->index].get();
    st_01_routenode = st_01_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7347220);
    SECTION("eva_to_station conversion of the 02 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_02_routenode;
    motis::station* st_02 = it->second;
    motis::station_node* st_02_stnode = schedule->station_nodes[st_02->index].get();
    st_02_routenode = st_02_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7190994);
    SECTION("eva_to_station conversion of the 03 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_03_routenode;
    motis::station* st_03 = it->second;
    motis::station_node* st_03_stnode = schedule->station_nodes[st_03->index].get();
    st_03_routenode = st_03_stnode->get_route_nodes()[0];

    SECTION("01 route node should not have a parent") {
        REQUIRE(ttr.parent_node(*st_01_routenode) == NULL);
    }

    SECTION("02 route node should have a parent equals to 01 route node") {
        REQUIRE(ttr.parent_node(*st_02_routenode) == st_01_routenode);
    }

    SECTION("03 route node should have a parent equals to 02 route node") {
        REQUIRE(ttr.parent_node(*st_03_routenode) == st_02_routenode);
    }
}

TEST_CASE("02:test case with a loop", "[railviz::timetable_retriever::parent_node]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;

    it = schedule->eva_to_station.find(5001307);
    SECTION("eva_to_station conversion of the 01 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_01_routenode;
    motis::station* st_01 = it->second;
    motis::station_node* st_01_stnode = schedule->station_nodes[st_01->index].get();
    st_01_routenode = st_01_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7347220);
    SECTION("eva_to_station conversion of the 02 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_02_routenode;
    motis::node* st_02_loop_routenode;
    motis::station* st_02 = it->second;
    motis::station_node* st_02_stnode = schedule->station_nodes[st_02->index].get();
    st_02_routenode = st_02_stnode->get_route_nodes()[0];
    st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

    it = schedule->eva_to_station.find(7190994);
    SECTION("eva_to_station conversion of the 03 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_03_routenode;
    motis::station* st_03 = it->second;
    motis::station_node* st_03_stnode = schedule->station_nodes[st_03->index].get();
    st_03_routenode = st_03_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(5386096);
    SECTION("eva_to_station conversion of the 04 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_04_routenode;
    motis::station* st_04 = it->second;
    motis::station_node* st_04_stnode = schedule->station_nodes[st_04->index].get();
    st_04_routenode = st_04_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7514434);
    SECTION("eva_to_station conversion of the 05 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_05_routenode;
    motis::station* st_05 = it->second;
    motis::station_node* st_05_stnode = schedule->station_nodes[st_05->index].get();
    st_05_routenode = st_05_stnode->get_route_nodes()[0];

    SECTION("01 route node should not have a parent") {
        REQUIRE(ttr.parent_node(*st_01_routenode) == NULL);
    }

    SECTION("02 route node should have a parent equals to 01 route node") {
        REQUIRE(ttr.parent_node(*st_02_routenode) == st_01_routenode);
    }

    SECTION("03 route node should have a parent equals to 02 route node") {
        REQUIRE(ttr.parent_node(*st_03_routenode) == st_02_routenode);
    }

    SECTION("04 route node should have a parent equals to 03 route node") {
        REQUIRE(ttr.parent_node(*st_04_routenode) == st_03_routenode);
    }

    SECTION("02 (loop path) route node should have a parent equals to 04 route node") {
        REQUIRE(ttr.parent_node(*st_02_loop_routenode) == st_04_routenode);
    }

    SECTION("05 route node should have a parent equals to 02 (loop path) route node") {
        REQUIRE(ttr.parent_node(*st_05_routenode) == st_02_loop_routenode);
    }
}

/**
 * Statement Coverage Tests of
 * const motis::node* child_node(const node &node) const
 */

TEST_CASE("03:test case without loop", "[railviz::timetable_retriever::child_node]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/01_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;

    it = schedule->eva_to_station.find(5001307);
    SECTION("eva_to_station conversion of the 01 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_01_routenode;
    motis::station* st_01 = it->second;
    motis::station_node* st_01_stnode = schedule->station_nodes[st_01->index].get();
    st_01_routenode = st_01_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7347220);
    SECTION("eva_to_station conversion of the 02 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_02_routenode;
    motis::station* st_02 = it->second;
    motis::station_node* st_02_stnode = schedule->station_nodes[st_02->index].get();
    st_02_routenode = st_02_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7190994);
    SECTION("eva_to_station conversion of the 03 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_03_routenode;
    motis::station* st_03 = it->second;
    motis::station_node* st_03_stnode = schedule->station_nodes[st_03->index].get();
    st_03_routenode = st_03_stnode->get_route_nodes()[0];

    SECTION("01 route node should have a child equals to 02 route node") {
        REQUIRE(ttr.child_node(*st_01_routenode) == st_02_routenode);
    }

    SECTION("02 route node should have a child equals to 03 route node") {
        REQUIRE(ttr.child_node(*st_02_routenode) == st_03_routenode);
    }

    SECTION("03 route node should not have any child") {
        REQUIRE(ttr.child_node(*st_03_routenode) == NULL);
    }
}

TEST_CASE("04:test case with a loop", "[railviz::timetable_retriever::child_node]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;

    it = schedule->eva_to_station.find(5001307);
    SECTION("eva_to_station conversion of the 01 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_01_routenode;
    motis::station* st_01 = it->second;
    motis::station_node* st_01_stnode = schedule->station_nodes[st_01->index].get();
    st_01_routenode = st_01_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7347220);
    SECTION("eva_to_station conversion of the 02 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_02_routenode;
    motis::node* st_02_loop_routenode;
    motis::station* st_02 = it->second;
    motis::station_node* st_02_stnode = schedule->station_nodes[st_02->index].get();
    st_02_routenode = st_02_stnode->get_route_nodes()[0];
    st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

    it = schedule->eva_to_station.find(7190994);
    SECTION("eva_to_station conversion of the 03 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_03_routenode;
    motis::station* st_03 = it->second;
    motis::station_node* st_03_stnode = schedule->station_nodes[st_03->index].get();
    st_03_routenode = st_03_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(5386096);
    SECTION("eva_to_station conversion of the 04 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_04_routenode;
    motis::station* st_04 = it->second;
    motis::station_node* st_04_stnode = schedule->station_nodes[st_04->index].get();
    st_04_routenode = st_04_stnode->get_route_nodes()[0];

    it = schedule->eva_to_station.find(7514434);
    SECTION("eva_to_station conversion of the 05 station should be successful") {
        REQUIRE(it != schedule->eva_to_station.end());
    }
    motis::node* st_05_routenode;
    motis::station* st_05 = it->second;
    motis::station_node* st_05_stnode = schedule->station_nodes[st_05->index].get();
    st_05_routenode = st_05_stnode->get_route_nodes()[0];

    SECTION("01 route node should have a child equals to 02 route node") {
        REQUIRE(ttr.child_node(*st_01_routenode) == st_02_routenode);
    }

    SECTION("02 route node should have a child equals to 03 route node") {
        REQUIRE(ttr.child_node(*st_02_routenode) == st_03_routenode);
    }

    SECTION("03 route node should have a child equals to 04 route node") {
        REQUIRE(ttr.child_node(*st_03_routenode) == st_04_routenode);
    }

    SECTION("04 route node should have a child equals to 02 (loop path) route node") {
        REQUIRE(ttr.child_node(*st_04_routenode) == st_02_loop_routenode);
    }

    SECTION("02 (loop path) route node should have a child equals to 05 route node") {
        REQUIRE(ttr.child_node(*st_02_loop_routenode) == st_05_routenode);
    }

    SECTION("05 route node should nit have aany child") {
        REQUIRE(ttr.child_node(*st_05_routenode) == NULL);
    }
}

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
