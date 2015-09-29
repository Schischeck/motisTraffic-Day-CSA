#include "catch/catch.hpp"

#include "motis/railviz/timetable_retriever.h"
#include "motis/loader/loader.h"

using namespace motis::railviz;

/**
 * The Tests operate on ../test_timetables/timetable_retriever_test/PLACEHOLDER/motis (relative to build path)
 */

/**
 * Statement Coverage Tests of
 * std::vector<motis::station_node const*> stations_on_route(const motis::node& node) const
 */

TEST_CASE("14:test case with a loop", "[railviz::timetable_retriever::stations_on_route]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
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

    std::vector<motis::station_node const*> ref_st_nodes_02 = { st_01_stnode,
                                                                st_02_stnode,
                                                                st_03_stnode,
                                                                st_04_stnode,
                                                                st_02_stnode,
                                                                st_05_stnode};
    std::vector<motis::station_node const*> res_st_nodes_02 = ttr.stations_on_route(*st_02_loop_routenode);

    SECTION("Station chain should be correct") {
        REQUIRE(ref_st_nodes_02 == res_st_nodes_02);
    }
}

/**
 * Statement Coverage Tests of
 * std::vector<route> get_routes_on_time(const motis::node& node, time time) const
 */

TEST_CASE("15:test case with a loop", "[railviz::timetable_retriever::get_routes_on_time]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
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

    std::vector<motis::railviz::route> res_st_node_02 =
            ttr.get_routes_on_time(*st_02_loop_routenode, 1439);

    SECTION("Route should be correct") {
        REQUIRE(std::get<0>(res_st_node_02.at(0).at(0)) == st_01_stnode);
        REQUIRE(std::get<1>(res_st_node_02.at(0).at(0)) == st_01_routenode);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(0))->d_time == 1439);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(0))->a_time == 1474);

        REQUIRE(std::get<0>(res_st_node_02.at(0).at(1)) == st_02_stnode);
        REQUIRE(std::get<1>(res_st_node_02.at(0).at(1)) == st_02_routenode);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(1))->d_time == 1476);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(1))->a_time == 1541);

        REQUIRE(std::get<0>(res_st_node_02.at(0).at(2)) == st_03_stnode);
        REQUIRE(std::get<1>(res_st_node_02.at(0).at(2)) == st_03_routenode);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(2))->d_time == 1543);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(2))->a_time == 1562);

        REQUIRE(std::get<0>(res_st_node_02.at(0).at(3)) == st_04_stnode);
        REQUIRE(std::get<1>(res_st_node_02.at(0).at(3)) == st_04_routenode);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(3))->d_time == 1564);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(3))->a_time == 1644);

        REQUIRE(std::get<0>(res_st_node_02.at(0).at(4)) == st_02_stnode);
        REQUIRE(std::get<1>(res_st_node_02.at(0).at(4)) == st_02_loop_routenode);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(4))->d_time == 1646);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(4))->a_time == 1732);

        REQUIRE(std::get<0>(res_st_node_02.at(0).at(5)) == st_05_stnode);
        REQUIRE(std::get<1>(res_st_node_02.at(0).at(5)) == st_05_routenode);
        REQUIRE(std::get<2>(res_st_node_02.at(0).at(5)) == NULL);
    }
}

/**
 * Statement Coverage Tests of
 * timetable ordered_timetable_for_station(const station_node& station) const
 */

TEST_CASE("13:test case with a loop", "[railviz::timetable_retriever::ordered_timetable_for_station]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
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

    motis::railviz::timetable res_tt_02 = ttr.ordered_timetable_for_station(*st_02_stnode);
    SECTION("Sorted Time table for station node 02 should be correct") {
        REQUIRE(std::get<0>(res_tt_02[0])->d_time == 36);
        REQUIRE(std::get<0>(res_tt_02[0])->a_time == 101);
        REQUIRE(std::get<1>(res_tt_02[0]) == st_02_stnode->get_route_nodes().at(0)->get_station());
        REQUIRE(std::get<2>(res_tt_02[0]) == ttr.child_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
        REQUIRE(std::get<3>(res_tt_02[0]) == st_05_routenode->get_station());
        REQUIRE(std::get<4>(res_tt_02[0]) == true);
        REQUIRE(std::get<5>(res_tt_02[0]) == st_02_stnode->get_route_nodes().at(0)->_route);
        REQUIRE(std::get<0>(res_tt_02[28])->d_time == 10116);
        REQUIRE(std::get<0>(res_tt_02[28])->a_time == 10181);
        REQUIRE(std::get<1>(res_tt_02[28]) == st_02_stnode->get_route_nodes().at(1)->get_station());
        REQUIRE(std::get<2>(res_tt_02[28]) == ttr.child_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
        REQUIRE(std::get<3>(res_tt_02[28]) == st_05_routenode->get_station());
        REQUIRE(std::get<4>(res_tt_02[28]) == true);
        REQUIRE(std::get<5>(res_tt_02[28]) == st_02_stnode->get_route_nodes().at(1)->_route);
    }
}

/**
 * Statement Coverage Tests of
 * void timetable_for_station_outgoing(const station_node& station, timetable& timetable_) const
 */

TEST_CASE("11:test case with a loop", "[railviz::timetable_retriever::timetable_for_station_outgoing]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
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
    SECTION("Time table for station node 02 should be correct") {
        ttr.timetable_for_station_outgoing(*st_02_stnode, res_tt_02);
        REQUIRE(std::get<0>(res_tt_02[0])->d_time == 36);
        REQUIRE(std::get<0>(res_tt_02[0])->a_time == 101);
        REQUIRE(std::get<1>(res_tt_02[0]) == st_02_stnode->get_route_nodes().at(0)->get_station());
        REQUIRE(std::get<2>(res_tt_02[0]) == ttr.child_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
        REQUIRE(std::get<3>(res_tt_02[0]) == st_05_routenode->get_station());
        REQUIRE(std::get<4>(res_tt_02[0]) == true);
        REQUIRE(std::get<5>(res_tt_02[0]) == st_02_stnode->get_route_nodes().at(0)->_route);
        REQUIRE(std::get<0>(res_tt_02[28])->d_time == 206);
        REQUIRE(std::get<0>(res_tt_02[28])->a_time == 292);
        REQUIRE(std::get<1>(res_tt_02[28]) == st_02_stnode->get_route_nodes().at(1)->get_station());
        REQUIRE(std::get<2>(res_tt_02[28]) == ttr.child_node(*(st_02_stnode->get_route_nodes().at(1)))->get_station());
        REQUIRE(std::get<3>(res_tt_02[28]) == st_05_routenode->get_station());
        REQUIRE(std::get<4>(res_tt_02[28]) == true);
        REQUIRE(std::get<5>(res_tt_02[28]) == st_02_stnode->get_route_nodes().at(1)->_route);
    }
}

/**
 * Statement Coverage Tests of
 * void timetable_for_station_incoming(const station_node& station, timetable& timetable_) const
 */

TEST_CASE("12:test case with a loop", "[railviz::timetable_retriever::timetable_for_station_incoming]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/02_test_set/motis");
    timetable_retriever ttr;
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
    SECTION("Time table for station node 02 should be correct") {
        ttr.timetable_for_station_incoming(*st_02_stnode, res_tt_02);
        REQUIRE(std::get<0>(res_tt_02[0])->d_time == 1439);
        REQUIRE(std::get<0>(res_tt_02[0])->a_time == 1474);
        REQUIRE(std::get<1>(res_tt_02[0]) == st_02_stnode->get_route_nodes().at(0)->get_station());
        REQUIRE(std::get<2>(res_tt_02[0]) == ttr.parent_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
        REQUIRE(std::get<3>(res_tt_02[0]) == st_01_routenode->get_station());
        REQUIRE(std::get<4>(res_tt_02[0]) == false);
        REQUIRE(std::get<5>(res_tt_02[0]) == st_02_stnode->get_route_nodes().at(0)->_route);
        REQUIRE(std::get<0>(res_tt_02[28])->d_time == 124);
        REQUIRE(std::get<0>(res_tt_02[28])->a_time == 204);
        REQUIRE(std::get<1>(res_tt_02[28]) == st_02_stnode->get_route_nodes().at(1)->get_station());
        REQUIRE(std::get<2>(res_tt_02[28]) == ttr.parent_node(*(st_02_stnode->get_route_nodes().at(1)))->get_station());
        REQUIRE(std::get<3>(res_tt_02[28]) == st_01_routenode->get_station());
        REQUIRE(std::get<4>(res_tt_02[28]) == false);
        REQUIRE(std::get<5>(res_tt_02[28]) == st_02_stnode->get_route_nodes().at(1)->_route);
    }
}

/**
 * Statement Coverage Tests of
 * const motis::node* parent_node(const node &node) const
 */

TEST_CASE("01:test case without loop", "[railviz::timetable_retriever::parent_node]") {
    auto schedule =
            motis::load_schedule("../test_timetables/timetable_retriever_test/01_test_set/motis");
    timetable_retriever ttr;
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
