#include "gtest/gtest.h"
#include "motis/core/common/date_util.h"
#include "motis/railviz/timetable_retriever.h"
#include "motis/loader/loader.h"
#include "motis/core/schedule/schedule.h"

using namespace motis::railviz;
using namespace motis;

/**
 * The Tests operate on
 * ../test_timetables/timetable_retriever_test/PLACEHOLDER/motis (relative to
 * build path)
 */

/**
 * Statement Coverage Tests of
 * std::vector<motis::station_node const*> stations_on_route(const motis::node&
 * node) const
 */

TEST(stations_on_route, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  motis::railviz::timetable ref_vector;

  it = schedule->eva_to_station.find("5001307");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  std::vector<motis::station_node const *> ref_st_nodes_02 = {
      st_01_stnode, st_02_stnode, st_03_stnode,
      st_04_stnode, st_02_stnode, st_05_stnode};
  std::vector<motis::station_node const *> res_st_nodes_02 =
      ttr.stations_on_route(*st_02_loop_routenode);

  assert(ref_st_nodes_02 == res_st_nodes_02);
}

/**
 * Statement Coverage Tests of
 * std::vector<route> get_routes_on_time(const motis::node& node, time time)
 * const
 */

TEST(routes_on_time, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  motis::railviz::timetable ref_vector;

  it = schedule->eva_to_station.find("5001307");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");

  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  std::vector<motis::railviz::route> res_st_node_02 =
      ttr.get_routes_on_time(*st_02_loop_routenode, 1439);

  assert(std::get<0>(res_st_node_02.at(0).at(0)) == st_01_stnode);
  assert(std::get<1>(res_st_node_02.at(0).at(0)) == st_01_routenode);
  assert(std::get<2>(res_st_node_02.at(0).at(0))->d_time == 1439);
  assert(std::get<2>(res_st_node_02.at(0).at(0))->a_time == 1474);

  assert(std::get<0>(res_st_node_02.at(0).at(1)) == st_02_stnode);
  assert(std::get<1>(res_st_node_02.at(0).at(1)) == st_02_routenode);
  assert(std::get<2>(res_st_node_02.at(0).at(1))->d_time == 1476);
  assert(std::get<2>(res_st_node_02.at(0).at(1))->a_time == 1541);

  assert(std::get<0>(res_st_node_02.at(0).at(2)) == st_03_stnode);
  assert(std::get<1>(res_st_node_02.at(0).at(2)) == st_03_routenode);
  assert(std::get<2>(res_st_node_02.at(0).at(2))->d_time == 1543);
  assert(std::get<2>(res_st_node_02.at(0).at(2))->a_time == 1562);

  assert(std::get<0>(res_st_node_02.at(0).at(3)) == st_04_stnode);
  assert(std::get<1>(res_st_node_02.at(0).at(3)) == st_04_routenode);
  assert(std::get<2>(res_st_node_02.at(0).at(3))->d_time == 1564);
  assert(std::get<2>(res_st_node_02.at(0).at(3))->a_time == 1644);

  assert(std::get<0>(res_st_node_02.at(0).at(4)) == st_02_stnode);
  assert(std::get<1>(res_st_node_02.at(0).at(4)) == st_02_loop_routenode);
  assert(std::get<2>(res_st_node_02.at(0).at(4))->d_time == 1646);
  assert(std::get<2>(res_st_node_02.at(0).at(4))->a_time == 1732);

  assert(std::get<0>(res_st_node_02.at(0).at(5)) == st_05_stnode);
  assert(std::get<1>(res_st_node_02.at(0).at(5)) == st_05_routenode);
  assert(std::get<2>(res_st_node_02.at(0).at(5)) == NULL);
}

/**
 * Statement Coverage Tests of
 * timetable ordered_timetable_for_station(const station_node& station) const
 */

TEST(timetable_for_station, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false,to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  motis::railviz::timetable ref_vector;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  motis::railviz::timetable res_tt_02 =
      ttr.ordered_timetable_for_station(*st_02_stnode);

  assert(std::get<0>(res_tt_02[0])->d_time == 36);
  assert(std::get<0>(res_tt_02[0])->a_time == 101);
  assert(std::get<1>(res_tt_02[0]) ==
              st_02_stnode->get_route_nodes().at(0)->get_station());
  assert(
      std::get<2>(res_tt_02[0]) ==
      ttr.child_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
  assert(std::get<3>(res_tt_02[0]) == st_05_routenode->get_station());
  assert(std::get<4>(res_tt_02[0]) == true);
  assert(std::get<5>(res_tt_02[0]) ==
              st_02_stnode->get_route_nodes().at(0)->_route);
  assert(std::get<0>(res_tt_02[28])->d_time == 10116);
  assert(std::get<0>(res_tt_02[28])->a_time == 10181);
  assert(std::get<1>(res_tt_02[28]) ==
              st_02_stnode->get_route_nodes().at(1)->get_station());
  assert(
      std::get<2>(res_tt_02[28]) ==
      ttr.child_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
  assert(std::get<3>(res_tt_02[28]) == st_05_routenode->get_station());
  assert(std::get<4>(res_tt_02[28]) == true);
  assert(std::get<5>(res_tt_02[28]) ==
              st_02_stnode->get_route_nodes().at(1)->_route);
}

/**
 * Statement Coverage Tests of
 * void timetable_for_station_outgoing(const station_node& station, timetable&
 * timetable_) const
 */

TEST(timetable_station_outgoing, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  motis::railviz::timetable ref_vector;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  motis::railviz::timetable res_tt_02;

  ttr.timetable_for_station_outgoing(*st_02_stnode, res_tt_02);
  assert(std::get<0>(res_tt_02[0])->d_time == 36);
  assert(std::get<0>(res_tt_02[0])->a_time == 101);
  assert(std::get<1>(res_tt_02[0]) ==
              st_02_stnode->get_route_nodes().at(0)->get_station());
  assert(
      std::get<2>(res_tt_02[0]) ==
      ttr.child_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
  assert(std::get<3>(res_tt_02[0]) == st_05_routenode->get_station());
  assert(std::get<4>(res_tt_02[0]) == true);
  assert(std::get<5>(res_tt_02[0]) ==
              st_02_stnode->get_route_nodes().at(0)->_route);
  assert(std::get<0>(res_tt_02[28])->d_time == 206);
  assert(std::get<0>(res_tt_02[28])->a_time == 292);
  assert(std::get<1>(res_tt_02[28]) ==
              st_02_stnode->get_route_nodes().at(1)->get_station());
  assert(
      std::get<2>(res_tt_02[28]) ==
      ttr.child_node(*(st_02_stnode->get_route_nodes().at(1)))->get_station());
  assert(std::get<3>(res_tt_02[28]) == st_05_routenode->get_station());
  assert(std::get<4>(res_tt_02[28]) == true);
  assert(std::get<5>(res_tt_02[28]) ==
              st_02_stnode->get_route_nodes().at(1)->_route);
}

/**
 * Statement Coverage Tests of
 * void timetable_for_station_incoming(const station_node& station, timetable&
 * timetable_) const
 */

TEST(timetable_station_incoming, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  motis::railviz::timetable ref_vector;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  motis::railviz::timetable res_tt_02;
  ttr.timetable_for_station_incoming(*st_02_stnode, res_tt_02);
  assert(std::get<0>(res_tt_02[0])->d_time == 1439);
  assert(std::get<0>(res_tt_02[0])->a_time == 1474);
  assert(std::get<1>(res_tt_02[0]) ==
              st_02_stnode->get_route_nodes().at(0)->get_station());
  assert(
      std::get<2>(res_tt_02[0]) ==
      ttr.parent_node(*(st_02_stnode->get_route_nodes().at(0)))->get_station());
  assert(std::get<3>(res_tt_02[0]) == st_01_routenode->get_station());
  assert(std::get<4>(res_tt_02[0]) == false);
  assert(std::get<5>(res_tt_02[0]) ==
              st_02_stnode->get_route_nodes().at(0)->_route);
  assert(std::get<0>(res_tt_02[28])->d_time == 124);
  assert(std::get<0>(res_tt_02[28])->a_time == 204);
  assert(std::get<1>(res_tt_02[28]) ==
              st_02_stnode->get_route_nodes().at(1)->get_station());
  assert(
      std::get<2>(res_tt_02[28]) ==
      ttr.parent_node(*(st_02_stnode->get_route_nodes().at(1)))->get_station());
  assert(std::get<3>(res_tt_02[28]) == st_01_routenode->get_station());
  assert(std::get<4>(res_tt_02[28]) == false);
  assert(std::get<5>(res_tt_02[28]) ==
              st_02_stnode->get_route_nodes().at(1)->_route);
}

/**
 * Statement Coverage Tests of
 * const motis::node* parent_node(const node &node) const
 */

TEST(parent_node, test_without_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "01_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_02_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  assert(ttr.parent_node(*st_01_routenode) == nullptr);

  assert(ttr.parent_node(*st_02_routenode) == st_01_routenode);

  assert(ttr.parent_node(*st_03_routenode) == st_02_routenode);
}

TEST(parent_node, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  assert(ttr.parent_node(*st_01_routenode) == nullptr);

  assert(ttr.parent_node(*st_02_routenode) == st_01_routenode);

  assert(ttr.parent_node(*st_03_routenode) == st_02_routenode);

  assert(ttr.parent_node(*st_04_routenode) == st_03_routenode);

  assert(ttr.parent_node(*st_02_loop_routenode) == st_04_routenode);

  assert(ttr.parent_node(*st_05_routenode) == st_02_loop_routenode);
}

/**
 * Statement Coverage Tests of
 * const motis::node* child_node(const node &node) const
 */

TEST(child_node, test_without_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "01_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  assert(ttr.child_node(*st_01_routenode) == st_02_routenode);

  assert(ttr.child_node(*st_02_routenode) == st_03_routenode);

  assert(ttr.child_node(*st_03_routenode) == nullptr);
}

TEST(child_node, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  assert(ttr.child_node(*st_01_routenode) == st_02_routenode);

  assert(ttr.child_node(*st_02_routenode) == st_03_routenode);

  assert(ttr.child_node(*st_03_routenode) == st_04_routenode);

  assert(ttr.child_node(*st_04_routenode) == st_02_loop_routenode);

  assert(ttr.child_node(*st_02_loop_routenode) == st_05_routenode);

  assert(ttr.child_node(*st_05_routenode) == nullptr);
}

/**
 * Statement Coverage Tests of
 * const motis::node* start_node_for_route( const motis::node& node_ ) const
 */

TEST(start_for_route, test_without_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "01_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");

  assert(it != schedule->eva_to_station.end());
  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");

  assert(it != schedule->eva_to_station.end());
  motis::node *st_02_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");

  assert(it != schedule->eva_to_station.end());
  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  assert(ttr.start_node_for_route(*st_01_routenode) == st_01_routenode);

  assert(ttr.start_node_for_route(*st_02_routenode) == st_01_routenode);

  assert(ttr.start_node_for_route(*st_03_routenode) == st_01_routenode);
}

TEST(start_for_route, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  assert(ttr.start_node_for_route(*st_01_routenode) == st_01_routenode);

  assert(ttr.start_node_for_route(*st_02_routenode) == st_01_routenode);

  assert(ttr.start_node_for_route(*st_03_routenode) == st_01_routenode);

  assert(ttr.start_node_for_route(*st_04_routenode) == st_01_routenode);

  assert(ttr.start_node_for_route(*st_02_loop_routenode) ==
              st_01_routenode);

  assert(ttr.start_node_for_route(*st_05_routenode) == st_01_routenode);
}

/**
 * Statement Coverage Tests of
 * const motis::node* end_node_for_route( const motis::node& node_ ) const
 */

TEST(end_for_route, test_without_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "01_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  assert(ttr.end_node_for_route(*st_01_routenode) == st_03_routenode);

  assert(ttr.end_node_for_route(*st_02_routenode) == st_03_routenode);

  assert(ttr.end_node_for_route(*st_03_routenode) == st_03_routenode);
}

TEST(end_for_route, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  assert(ttr.end_node_for_route(*st_01_routenode) == st_05_routenode);

  assert(ttr.end_node_for_route(*st_02_routenode) == st_05_routenode);

  assert(ttr.end_node_for_route(*st_03_routenode) == st_05_routenode);

  assert(ttr.end_node_for_route(*st_04_routenode) == st_05_routenode);

  assert(ttr.end_node_for_route(*st_02_loop_routenode) == st_05_routenode);

  assert(ttr.end_node_for_route(*st_05_routenode) == st_05_routenode);
}

/**
 * Statement Coverage Tests of
 * std::vector<motis::time> get_route_departure_times( const motis::node& node_
 * ) const
 */

TEST(route_departure_times, test_without_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "01_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  std::vector<motis::time> ref_vector = {
      1439,  2879,  4319,  5759,  7199,  8639,  10079, 11519, 12959, 14399,
      15839, 17279, 18719, 20159, 21599, 23039, 24479, 25919, 27359, 28799,
      30239, 31679, 33119, 34559, 35999, 37439, 38879, 40319};

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_02_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());
  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  assert(ttr.get_route_departure_times(*st_01_routenode) == ref_vector);

  assert(ttr.get_route_departure_times(*st_02_routenode) == ref_vector);

  assert(ttr.get_route_departure_times(*st_03_routenode) == ref_vector);
}

TEST(route_departure_times, test_with_loop) {
  auto schedule = loader::load_schedule("modules/railviz/test/test_timetables/"
                                        "02_test_set/",
                                        true, false, to_unix_time(2015, 11, 21),
                                        to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station *>::iterator it;
  std::vector<motis::time> ref_vector = {
      1439,  2879,  4319,  5759,  7199,  8639,  10079, 11519, 12959, 14399,
      15839, 17279, 18719, 20159, 21599, 23039, 24479, 25919, 27359, 28799,
      30239, 31679, 33119, 34559, 35999, 37439, 38879, 40319};

  it = schedule->eva_to_station.find("5001307");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_01_routenode;
  motis::station *st_01 = it->second;
  motis::station_node *st_01_stnode =
      schedule->station_nodes[st_01->index].get();
  st_01_routenode = st_01_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_02_routenode;
  motis::node *st_02_loop_routenode;
  motis::station *st_02 = it->second;
  motis::station_node *st_02_stnode =
      schedule->station_nodes[st_02->index].get();
  st_02_routenode = st_02_stnode->get_route_nodes()[0];
  st_02_loop_routenode = st_02_stnode->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_03_routenode;
  motis::station *st_03 = it->second;
  motis::station_node *st_03_stnode =
      schedule->station_nodes[st_03->index].get();
  st_03_routenode = st_03_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("5386096");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_04_routenode;
  motis::station *st_04 = it->second;
  motis::station_node *st_04_stnode =
      schedule->station_nodes[st_04->index].get();
  st_04_routenode = st_04_stnode->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  assert(it != schedule->eva_to_station.end());

  motis::node *st_05_routenode;
  motis::station *st_05 = it->second;
  motis::station_node *st_05_stnode =
      schedule->station_nodes[st_05->index].get();
  st_05_routenode = st_05_stnode->get_route_nodes()[0];

  assert(ttr.get_route_departure_times(*st_01_routenode) == ref_vector);

  assert(ttr.get_route_departure_times(*st_02_routenode) == ref_vector);

  assert(ttr.get_route_departure_times(*st_03_routenode) == ref_vector);

  assert(ttr.get_route_departure_times(*st_04_routenode) == ref_vector);

  assert(ttr.get_route_departure_times(*st_02_loop_routenode) ==
              ref_vector);

  assert(ttr.get_route_departure_times(*st_05_routenode) == ref_vector);
}
