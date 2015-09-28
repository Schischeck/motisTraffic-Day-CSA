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
 * void timetable_for_station_outgoing(const station_node& station, timetable& timetable_) const
 */

bool timetable_comparator (const motis::railviz::timetable& left, const motis::railviz::timetable& right) {
    if (left.size() != right.size()) return false;
    for (int i = 0; i < left.size(); i++) {
        if (left.at(i) != right.at(i)) {

            std::cout << "left equals left? : " << (left.at(i) == left.at(i)) << std::endl;
            std::cout << "left equals right? : " << (left.at(i) == right.at(i)) << std::endl;

            return false;
        }
    }
    return true;
}

TEST_CASE("11:test case with a loop", "[railviz::timetable_retriever::timetable_for_station_outgoing]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;
    motis::railviz::timetable ref_vector;

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

    motis::railviz::timetable res_tt_02;
    motis::railviz::timetable ref_tt_02;
    for (int i = 0; i < 2; i++) { //go through route_nodes
        for (int j = 0; j < 2; j++) { //go through edges
            for (const motis::light_connection& l : st_02_stnode->get_route_nodes().at(i)->_edges[j]._m._route_edge._conns) {
                ref_tt_02.push_back(motis::railviz::timetable_entry(
                                        &l,
                                        st_02_stnode->get_route_nodes().at(i)->get_station(),
                                        ttr.child_node(*(st_02_stnode->get_route_nodes().at(i)))->get_station(),
                                        st_05_routenode->get_station(),
                                        true,
                                        st_02_stnode->get_route_nodes().at(i)->_route
                                        ));
            }
        }
    }

    SECTION("Time table for station node 02 should be eqal to ref_tt_02") {
        ttr.timetable_for_station_outgoing(*st_02_stnode, res_tt_02);
        REQUIRE(timetable_comparator(res_tt_02, ref_tt_02));
    }
}


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
        REQUIRE(ttr.parent_node(*st_01_routenode) == nullptr);
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
        REQUIRE(ttr.parent_node(*st_01_routenode) == nullptr);
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
        REQUIRE(ttr.child_node(*st_03_routenode) == nullptr);
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
        REQUIRE(ttr.child_node(*st_05_routenode) == nullptr);
    }
}

/**
 * Statement Coverage Tests of
 * const motis::node* start_node_for_route( const motis::node& node_ ) const
 */

TEST_CASE("05:test case without loop", "[railviz::timetable_retriever::start_node_for_route]") {
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

    SECTION("01 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_01_routenode) == st_01_routenode);
    }

    SECTION("02 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_02_routenode) == st_01_routenode);
    }

    SECTION("03 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_03_routenode) == st_01_routenode);
    }
}

TEST_CASE("06:test case with a loop", "[railviz::timetable_retriever::start_node_for_route]") {
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

    SECTION("01 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_01_routenode) == st_01_routenode);
    }

    SECTION("02 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_02_routenode) == st_01_routenode);
    }

    SECTION("03 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_03_routenode) == st_01_routenode);
    }

    SECTION("04 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_04_routenode) == st_01_routenode);
    }

    SECTION("02 (loop path) route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_02_loop_routenode) == st_01_routenode);
    }

    SECTION("05 route node should belong to a route with the start node 01") {
        REQUIRE(ttr.start_node_for_route(*st_05_routenode) == st_01_routenode);
    }
}

/**
 * Statement Coverage Tests of
 * const motis::node* end_node_for_route( const motis::node& node_ ) const
 */

TEST_CASE("07:test case without loop", "[railviz::timetable_retriever::end_node_for_route]") {
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

    SECTION("01 route node should belong to a route with the end node 03") {
        REQUIRE(ttr.end_node_for_route(*st_01_routenode) == st_03_routenode);
    }

    SECTION("02 route node should belong to a route with the end node 03") {
        REQUIRE(ttr.end_node_for_route(*st_02_routenode) == st_03_routenode);
    }

    SECTION("03 route node should belong to a route with the end node 03") {
        REQUIRE(ttr.end_node_for_route(*st_03_routenode) == st_03_routenode);
    }
}

TEST_CASE("08:test case with a loop", "[railviz::timetable_retriever::end_node_for_route]") {
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

    SECTION("01 route node should belong to a route with the end node 05") {
        REQUIRE(ttr.end_node_for_route(*st_01_routenode) == st_05_routenode);
    }

    SECTION("02 route node should belong to a route with the end node 05") {
        REQUIRE(ttr.end_node_for_route(*st_02_routenode) == st_05_routenode);
    }

    SECTION("03 route node should belong to a route with the start node 05") {
        REQUIRE(ttr.end_node_for_route(*st_03_routenode) == st_05_routenode);
    }

    SECTION("04 route node should belong to a route with the start node 05") {
        REQUIRE(ttr.end_node_for_route(*st_04_routenode) == st_05_routenode);
    }

    SECTION("02 (loop path) route node should belong to a route with the end node 05") {
        REQUIRE(ttr.end_node_for_route(*st_02_loop_routenode) == st_05_routenode);
    }

    SECTION("05 route node should belong to a route with the end node 05") {
        REQUIRE(ttr.end_node_for_route(*st_05_routenode) == st_05_routenode);
    }
}

/**
 * Statement Coverage Tests of
 * std::vector<motis::time> get_route_departure_times( const motis::node& node_ ) const
 */

TEST_CASE("09:test case without loop", "[railviz::timetable_retriever::get_route_depature_times]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/01_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;
    std::vector<motis::time> ref_vector = { 1439,  2879,
                                            4319,  5759,
                                            7199,  8639,
                                            10079, 11519,
                                            12959, 14399,
                                            15839, 17279,
                                            18719, 20159,
                                            21599, 23039,
                                            24479, 25919,
                                            27359, 28799,
                                            30239, 31679,
                                            33119, 34559,
                                            35999, 37439,
                                            38879, 40319 };

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

    SECTION("01 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_01_routenode) == ref_vector);
    }

    SECTION("02 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_02_routenode) == ref_vector);
    }

    SECTION("03 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_03_routenode) == ref_vector);
    }
}

TEST_CASE("10:test case with a loop", "[railviz::timetable_retriever::get_route_depature_times]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
    ttr.init(*(schedule.get()));
    std::map<int, motis::station*>::iterator it;
    std::vector<motis::time> ref_vector = { 1439,  2879,
                                            4319,  5759,
                                            7199,  8639,
                                            10079, 11519,
                                            12959, 14399,
                                            15839, 17279,
                                            18719, 20159,
                                            21599, 23039,
                                            24479, 25919,
                                            27359, 28799,
                                            30239, 31679,
                                            33119, 34559,
                                            35999, 37439,
                                            38879, 40319 };

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

    SECTION("01 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_01_routenode) == ref_vector);
    }

    SECTION("02 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_02_routenode) == ref_vector);
    }

    SECTION("03 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_03_routenode) == ref_vector);
    }

    SECTION("04 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_04_routenode) == ref_vector);
    }

    SECTION("02 (loop path) route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_02_loop_routenode) == ref_vector);
    }

    SECTION("05 route node should belong to a route with the ref_vector of d_time's") {
        REQUIRE(ttr.get_route_departure_times(*st_05_routenode) == ref_vector);
    }
}
