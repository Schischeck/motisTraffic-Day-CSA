#include "gtest/gtest.h"
#include "motis/core/common/date_util.h"
#include "motis/railviz/timetable_retriever.h"
#include "motis/loader/loader.h"
#include "motis/core/schedule/schedule.h"

using namespace motis::railviz;
using namespace motis;

TEST(railviz_stations_on_route, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_01 = schedule->station_nodes[it->second->index].get();

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_02 = schedule->station_nodes[it->second->index].get();
  auto route_node_02_loop = station_node_02->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7190994");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_03 = schedule->station_nodes[it->second->index].get();

  it = schedule->eva_to_station.find("5386096");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_04 = schedule->station_nodes[it->second->index].get();

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_05 = schedule->station_nodes[it->second->index].get();

  std::vector<motis::station_node const*> ref_route = {
      station_node_01, station_node_02, station_node_03,
      station_node_04, station_node_02, station_node_05};
  auto route = ttr.stations_on_route(*route_node_02_loop);

  EXPECT_EQ(ref_route, route);
}

TEST(railviz_routes_on_time, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  std::vector<motis::time> ref_vector_times = {
      offset + 0,   offset + 35,  offset + 37,  offset + 102, offset + 104,
      offset + 123, offset + 125, offset + 205, offset + 207, offset + 293};

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_02 = schedule->station_nodes[it->second->index].get();
  auto route_node_02_loop = station_node_02->get_route_nodes()[1];

  it = schedule->eva_to_station.find("5386096");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_04 = schedule->station_nodes[it->second->index].get();
  auto route_node_04 = station_node_04->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_05 = schedule->station_nodes[it->second->index].get();
  auto route_node_05 = station_node_05->get_route_nodes()[0];

  auto const& routes =
      ttr.get_routes_on_time(*route_node_02_loop, ref_vector_times[0]);
  ASSERT_FALSE(routes.empty());
  ASSERT_EQ(routes.size(), 1);
  auto const& route = routes.at(0);

  EXPECT_EQ(ref_vector_times[0], std::get<2>(route[0])->d_time);
  EXPECT_EQ(ref_vector_times[1], std::get<2>(route[0])->a_time);
  EXPECT_EQ(ref_vector_times[2], std::get<2>(route[1])->d_time);
  EXPECT_EQ(ref_vector_times[3], std::get<2>(route[1])->a_time);
  EXPECT_EQ(ref_vector_times[4], std::get<2>(route[2])->d_time);
  EXPECT_EQ(ref_vector_times[5], std::get<2>(route[2])->a_time);

  EXPECT_EQ(station_node_04, std::get<0>(route[3]));
  EXPECT_EQ(route_node_04, std::get<1>(route[3]));
  EXPECT_EQ(ref_vector_times[6], std::get<2>(route[3])->d_time);
  EXPECT_EQ(ref_vector_times[7], std::get<2>(route[3])->a_time);

  EXPECT_EQ(station_node_02, std::get<0>(route[4]));
  EXPECT_EQ(route_node_02_loop, std::get<1>(route[4]));
  EXPECT_EQ(ref_vector_times[8], std::get<2>(route[4])->d_time);
  EXPECT_EQ(ref_vector_times[9], std::get<2>(route[4])->a_time);

  EXPECT_EQ(station_node_05, std::get<0>(route[5]));
  EXPECT_EQ(route_node_05, std::get<1>(route[5]));
  EXPECT_EQ(NULL, std::get<2>(route[5]));
}

TEST(railviz_timetable_for_station, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));

  timetable_retriever ttr;
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 =
      schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_02 = schedule->station_nodes[it->second->index].get();
  auto route_node_02 = station_node_02->get_route_nodes()[0];
  auto route_node_02_loop = station_node_02->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_05 =
      schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  motis::railviz::timetable ordered_timetable =
      ttr.ordered_timetable_for_station(*station_node_02);

  EXPECT_EQ(offset, std::get<0>(ordered_timetable[0])->d_time);
  EXPECT_EQ(offset + 35, std::get<0>(ordered_timetable[0])->a_time);
  EXPECT_EQ(route_node_02->get_station(), std::get<1>(ordered_timetable[0]));
  EXPECT_EQ(ttr.parent_node(*route_node_02)->get_station(),
            std::get<2>(ordered_timetable[0]));
  EXPECT_EQ(route_node_01->get_station(), std::get<3>(ordered_timetable[0]));
  EXPECT_FALSE(std::get<4>(ordered_timetable[0]));
  EXPECT_EQ(route_node_02->_route, std::get<5>(ordered_timetable[0]));

  EXPECT_EQ(offset + 207, std::get<0>(ordered_timetable[18])->d_time);
  EXPECT_EQ(offset + 293, std::get<0>(ordered_timetable[18])->a_time);
  EXPECT_EQ(route_node_02_loop->get_station(),
            std::get<1>(ordered_timetable[18]));
  EXPECT_EQ(ttr.child_node(*route_node_02_loop)->get_station(),
            std::get<2>(ordered_timetable[18]));
  EXPECT_EQ(route_node_05->get_station(), std::get<3>(ordered_timetable[18]));
  EXPECT_TRUE(std::get<4>(ordered_timetable[18]));
  EXPECT_EQ(route_node_02_loop->_route, std::get<5>(ordered_timetable[18]));
}

TEST(railviz_timetable_station_outgoing, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_02 = schedule->station_nodes[it->second->index].get();
  auto route_node_02 = station_node_02->get_route_nodes()[0];
  auto route_node_02_loop = station_node_02->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_05 =
      schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  motis::railviz::timetable timetable_outgoing;
  ttr.timetable_for_station_outgoing(*station_node_02, timetable_outgoing);
  ASSERT_FALSE(timetable_outgoing.empty());

  EXPECT_EQ(offset + 37, std::get<0>(timetable_outgoing[0])->d_time);
  EXPECT_EQ(offset + 102, std::get<0>(timetable_outgoing[0])->a_time);
  EXPECT_EQ(route_node_02->get_station(), std::get<1>(timetable_outgoing[0]));
  EXPECT_EQ(ttr.child_node(*route_node_02)->get_station(),
            std::get<2>(timetable_outgoing[0]));
  EXPECT_EQ(route_node_05->get_station(), std::get<3>(timetable_outgoing[0]));
  EXPECT_TRUE(std::get<4>(timetable_outgoing[0]));
  EXPECT_EQ(route_node_02->_route, std::get<5>(timetable_outgoing[0]));

  EXPECT_EQ(offset + 207, std::get<0>(timetable_outgoing[6])->d_time);
  EXPECT_EQ(offset + 293, std::get<0>(timetable_outgoing[6])->a_time);
  EXPECT_EQ(route_node_02_loop->get_station(),
            std::get<1>(timetable_outgoing[6]));
  EXPECT_EQ(ttr.child_node(*route_node_02_loop)->get_station(),
            std::get<2>(timetable_outgoing[6]));
  EXPECT_EQ(route_node_05->get_station(), std::get<3>(timetable_outgoing[6]));
  EXPECT_TRUE(std::get<4>(timetable_outgoing[6]));
  EXPECT_EQ(route_node_02_loop->_route, std::get<5>(timetable_outgoing[6]));
}

TEST(railviz_timetable_station_incoming, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto station_node_02 = schedule->station_nodes[it->second->index].get();
  auto route_node_02 = station_node_02->get_route_nodes()[0];
  auto route_node_02_loop = station_node_02->get_route_nodes()[1];

  motis::railviz::timetable timetable_ingoing;
  ttr.timetable_for_station_incoming(*station_node_02, timetable_ingoing);
  ASSERT_FALSE(timetable_ingoing.empty());

  EXPECT_EQ(offset, std::get<0>(timetable_ingoing[0])->d_time);
  EXPECT_EQ(offset + 35, std::get<0>(timetable_ingoing[0])->a_time);
  EXPECT_EQ(route_node_02->get_station(), std::get<1>(timetable_ingoing[0]));
  EXPECT_EQ(ttr.parent_node(*route_node_02)->get_station(),
            std::get<2>(timetable_ingoing[0]));
  EXPECT_EQ(route_node_01->get_station(), std::get<3>(timetable_ingoing[0]));
  EXPECT_FALSE(std::get<4>(timetable_ingoing[0]));
  EXPECT_EQ(route_node_02->_route, std::get<5>(timetable_ingoing[0]));

  EXPECT_EQ(offset + 125, std::get<0>(timetable_ingoing[6])->d_time);
  EXPECT_EQ(offset + 205, std::get<0>(timetable_ingoing[6])->a_time);
  EXPECT_EQ(route_node_02_loop->get_station(),
            std::get<1>(timetable_ingoing[6]));
  EXPECT_EQ(ttr.parent_node(*route_node_02_loop)->get_station(),
            std::get<2>(timetable_ingoing[6]));
  EXPECT_EQ(route_node_01->get_station(), std::get<3>(timetable_ingoing[6]));
  EXPECT_FALSE(std::get<4>(timetable_ingoing[6]));
  EXPECT_EQ(route_node_02_loop->_route, std::get<5>(timetable_ingoing[6]));
}

TEST(railviz_parent_node, test_without_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_03 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(nullptr, ttr.parent_node(*route_node_01));
  EXPECT_EQ(route_node_01, ttr.parent_node(*route_node_02));
  EXPECT_EQ(route_node_02, ttr.parent_node(*route_node_03));
}

TEST(railviz_parent_node, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];
  auto route_node_02_loop = schedule->station_nodes[it->second->index].get()->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_05 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(nullptr, ttr.parent_node(*route_node_01));
  EXPECT_EQ(route_node_01, ttr.parent_node(*route_node_02));
  EXPECT_EQ(route_node_02_loop, ttr.parent_node(*route_node_05));
}

TEST(railviz_child_node, test_without_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_03 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(route_node_02, ttr.child_node(*route_node_01));
  EXPECT_EQ(route_node_03, ttr.child_node(*route_node_02));
  EXPECT_EQ(nullptr, ttr.child_node(*route_node_03));
}

TEST(railviz_child_node, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];
  auto route_node_02_loop = schedule->station_nodes[it->second->index].get()->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_05 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(route_node_02, ttr.child_node(*route_node_01));
  EXPECT_EQ(route_node_05, ttr.child_node(*route_node_02_loop));
  EXPECT_EQ(nullptr, ttr.child_node(*route_node_05));
}

TEST(railviz_start_for_route, test_without_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_03 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_01));
  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_02));
  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_03));
}

TEST(railviz_start_for_route, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];
  auto route_node_02_loop = schedule->station_nodes[it->second->index].get()->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_05 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_01));
  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_02));
  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_02_loop));
  EXPECT_EQ(route_node_01, ttr.start_node_for_route(*route_node_05));
}

TEST(railviz_end_for_route, test_without_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule/",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7190994");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_03 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(route_node_03, ttr.end_node_for_route(*route_node_01));
  EXPECT_EQ(route_node_03, ttr.end_node_for_route(*route_node_02));
  EXPECT_EQ(route_node_03, ttr.end_node_for_route(*route_node_03));
}

TEST(railviz_end_for_route, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  std::map<std::string, motis::station*>::iterator it;

  it = schedule->eva_to_station.find("5001307");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_01 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  it = schedule->eva_to_station.find("7347220");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_02 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];
  auto route_node_02_loop = schedule->station_nodes[it->second->index].get()->get_route_nodes()[1];

  it = schedule->eva_to_station.find("7514434");
  ASSERT_NE(schedule->eva_to_station.end(), it);
  auto route_node_05 = schedule->station_nodes[it->second->index].get()->get_route_nodes()[0];

  EXPECT_EQ(route_node_05, ttr.end_node_for_route(*route_node_01));
  EXPECT_EQ(route_node_05, ttr.end_node_for_route(*route_node_02));
  EXPECT_EQ(route_node_05, ttr.end_node_for_route(*route_node_02_loop));
  EXPECT_EQ(route_node_05, ttr.end_node_for_route(*route_node_05));
}

TEST(railviz_route_departure_times, test_without_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  std::vector<motis::time> ref_vector = {offset + 0,  offset + 10, offset + 20,
                                         offset + 30, offset + 40, offset + 50};
  auto const& it = schedule->eva_to_station.find("7347220");
  ASSERT_TRUE(it != begin(schedule->eva_to_station));
  auto const& station_node = *schedule->station_nodes[it->second->index].get();
  auto const& route_node = *station_node.get_route_nodes()[0];
  EXPECT_EQ(ref_vector, ttr.get_route_departure_times(route_node));
}

TEST(railviz_route_departure_times, test_with_loop) {
  auto schedule = loader::load_schedule(
      "modules/railviz/test/test_timetables/"
      "schedule_loop",
      true, false, to_unix_time(2015, 11, 21), to_unix_time(2015, 11, 22));
  timetable_retriever ttr;
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  std::vector<motis::time> ref_vector = {offset + 0,  offset + 10, offset + 20,
                                         offset + 30, offset + 40, offset + 50};
  auto const& it = schedule->eva_to_station.find("5386096");
  EXPECT_TRUE(it != schedule->eva_to_station.end());
  auto const& station_node = *schedule->station_nodes[it->second->index].get();
  auto const& route_node = *station_node.get_route_nodes()[0];
  EXPECT_EQ(ref_vector, ttr.get_route_departure_times(route_node));
}
