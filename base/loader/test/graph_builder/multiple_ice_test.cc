#include "./graph_builder_test.h"

#include "motis/core/common/date_util.h"

using std::get;

namespace motis {
namespace loader {

class loader_graph_builder_multiple_ice : public loader_graph_builder_test {
public:
  loader_graph_builder_multiple_ice()
      : loader_graph_builder_test(
            "multiple-ice-files", to_unix_time(2015, 10, 25),
            to_unix_time(2015, 10, 25) + 2 * MINUTES_A_DAY * 60) {}
};

TEST_F(loader_graph_builder_multiple_ice, eva_num) {
  auto& stations = sched_->eva_to_station;
  EXPECT_STREQ("8000013", stations["8000013"]->eva_nr.c_str());
  EXPECT_STREQ("8000025", stations["8000025"]->eva_nr.c_str());
  EXPECT_STREQ("8000078", stations["8000078"]->eva_nr.c_str());
  EXPECT_STREQ("8000122", stations["8000122"]->eva_nr.c_str());
  EXPECT_STREQ("8000228", stations["8000228"]->eva_nr.c_str());
  EXPECT_STREQ("8000260", stations["8000260"]->eva_nr.c_str());
  EXPECT_STREQ("8000261", stations["8000261"]->eva_nr.c_str());
  EXPECT_STREQ("8000284", stations["8000284"]->eva_nr.c_str());
  EXPECT_STREQ("8001844", stations["8001844"]->eva_nr.c_str());
  EXPECT_STREQ("8004158", stations["8004158"]->eva_nr.c_str());
  EXPECT_STREQ("8010101", stations["8010101"]->eva_nr.c_str());
  EXPECT_STREQ("8010205", stations["8010205"]->eva_nr.c_str());
  EXPECT_STREQ("8010222", stations["8010222"]->eva_nr.c_str());
  EXPECT_STREQ("8010240", stations["8010240"]->eva_nr.c_str());
  EXPECT_STREQ("8010309", stations["8010309"]->eva_nr.c_str());
  EXPECT_STREQ("8011102", stations["8011102"]->eva_nr.c_str());
  EXPECT_STREQ("8011113", stations["8011113"]->eva_nr.c_str());
  EXPECT_STREQ("8011956", stations["8011956"]->eva_nr.c_str());
  EXPECT_STREQ("8098160", stations["8098160"]->eva_nr.c_str());
}

TEST_F(loader_graph_builder_multiple_ice, simple_test) {
  auto& stations = sched_->eva_to_station;
  ASSERT_STREQ("Augsburg Hbf", stations["8000013"]->name.c_str());
  ASSERT_STREQ("Bamberg", stations["8000025"]->name.c_str());
  ASSERT_STREQ("Donauwörth", stations["8000078"]->name.c_str());
  ASSERT_STREQ("Treuchtlingen", stations["8000122"]->name.c_str());
  ASSERT_STREQ("Lichtenfels", stations["8000228"]->name.c_str());
  ASSERT_STREQ("Würzburg Hbf", stations["8000260"]->name.c_str());
  ASSERT_STREQ("München Hbf", stations["8000261"]->name.c_str());
  ASSERT_STREQ("Nürnberg Hbf", stations["8000284"]->name.c_str());
  ASSERT_STREQ("Erlangen", stations["8001844"]->name.c_str());
  ASSERT_STREQ("München-Pasing", stations["8004158"]->name.c_str());
  ASSERT_STREQ("Erfurt Hbf", stations["8010101"]->name.c_str());
  ASSERT_STREQ("Leipzig Hbf", stations["8010205"]->name.c_str());
  ASSERT_STREQ("Lutherstadt Wittenberg", stations["8010222"]->name.c_str());
  ASSERT_STREQ("Naumburg(Saale)Hbf", stations["8010240"]->name.c_str());
  ASSERT_STREQ("Saalfeld(Saale)", stations["8010309"]->name.c_str());
  ASSERT_STREQ("Berlin Gesundbrunnen", stations["8011102"]->name.c_str());
  ASSERT_STREQ("Berlin Südkreuz", stations["8011113"]->name.c_str());
  ASSERT_STREQ("Jena Paradies", stations["8011956"]->name.c_str());
  ASSERT_STREQ("Berlin Hbf (tief)", stations["8098160"]->name.c_str());
}

TEST_F(loader_graph_builder_multiple_ice, coordinates) {
  auto& stations = sched_->eva_to_station;

  ASSERT_FLOAT_EQ(48.3654410, stations["8000013"]->width);
  ASSERT_FLOAT_EQ(49.9007590, stations["8000025"]->width);
  ASSERT_FLOAT_EQ(48.7140260, stations["8000078"]->width);
  ASSERT_FLOAT_EQ(48.9612670, stations["8000122"]->width);
  ASSERT_FLOAT_EQ(50.1464520, stations["8000228"]->width);
  ASSERT_FLOAT_EQ(49.8017960, stations["8000260"]->width);
  ASSERT_FLOAT_EQ(48.1402320, stations["8000261"]->width);
  ASSERT_FLOAT_EQ(49.4456160, stations["8000284"]->width);
  ASSERT_FLOAT_EQ(49.5958950, stations["8001844"]->width);
  ASSERT_FLOAT_EQ(48.1498960, stations["8004158"]->width);
  ASSERT_FLOAT_EQ(50.9725510, stations["8010101"]->width);
  ASSERT_FLOAT_EQ(51.3465490, stations["8010205"]->width);
  ASSERT_FLOAT_EQ(51.8675310, stations["8010222"]->width);
  ASSERT_FLOAT_EQ(51.1630710, stations["8010240"]->width);
  ASSERT_FLOAT_EQ(50.6503160, stations["8010309"]->width);
  ASSERT_FLOAT_EQ(52.5489630, stations["8011102"]->width);
  ASSERT_FLOAT_EQ(52.4750470, stations["8011113"]->width);
  ASSERT_FLOAT_EQ(50.9248560, stations["8011956"]->width);
  ASSERT_FLOAT_EQ(52.5255920, stations["8098160"]->width);

  ASSERT_FLOAT_EQ(10.8855700, stations["8000013"]->length);
  ASSERT_FLOAT_EQ(10.8994890, stations["8000025"]->length);
  ASSERT_FLOAT_EQ(10.7714430, stations["8000078"]->length);
  ASSERT_FLOAT_EQ(10.9081590, stations["8000122"]->length);
  ASSERT_FLOAT_EQ(11.0594720, stations["8000228"]->length);
  ASSERT_FLOAT_EQ(9.93578000, stations["8000260"]->length);
  ASSERT_FLOAT_EQ(11.5583350, stations["8000261"]->length);
  ASSERT_FLOAT_EQ(11.0829890, stations["8000284"]->length);
  ASSERT_FLOAT_EQ(11.0019080, stations["8001844"]->length);
  ASSERT_FLOAT_EQ(11.4614850, stations["8004158"]->length);
  ASSERT_FLOAT_EQ(11.0384990, stations["8010101"]->length);
  ASSERT_FLOAT_EQ(12.3833360, stations["8010205"]->length);
  ASSERT_FLOAT_EQ(12.6620150, stations["8010222"]->length);
  ASSERT_FLOAT_EQ(11.7969840, stations["8010240"]->length);
  ASSERT_FLOAT_EQ(11.3749870, stations["8010309"]->length);
  ASSERT_FLOAT_EQ(13.3885130, stations["8011102"]->length);
  ASSERT_FLOAT_EQ(13.3653190, stations["8011113"]->length);
  ASSERT_FLOAT_EQ(11.5874610, stations["8011956"]->length);
  ASSERT_FLOAT_EQ(13.3695450, stations["8098160"]->length);
}

TEST_F(loader_graph_builder_multiple_ice, route_nodes) {
  EXPECT_EQ(1, sched_->route_index_to_first_route_node.size());

  for (auto const& first_route_node : sched_->route_index_to_first_route_node) {
    ASSERT_TRUE(first_route_node->is_route_node());

    auto station_id = first_route_node->get_station()->_id;
    auto station_eva = sched_->stations[station_id]->eva_nr;

    EXPECT_TRUE(station_eva == "8000284" || station_eva == "8000261");

    if (station_eva == "8000284") {
      ASSERT_EQ(1, first_route_node->_incoming_edges.size());
      ASSERT_EQ(2, first_route_node->_edges.size());

      ASSERT_EQ(first_route_node->_incoming_edges[0]->_from,
                first_route_node->get_station());
      ASSERT_EQ(first_route_node->get_station(),
                first_route_node[0]._edges[0]._to);

      ASSERT_EQ(edge::FOOT_EDGE, first_route_node->_incoming_edges[0]->type());
      ASSERT_EQ(edge::ROUTE_EDGE, first_route_node->_edges[1].type());
      ASSERT_EQ(edge::FOOT_EDGE, first_route_node->_edges[0].type());

      auto next_route_node = first_route_node->_edges[1]._to;
      ASSERT_TRUE(next_route_node->is_route_node());

      ASSERT_STREQ("8000260",
                   sched_->stations[next_route_node->get_station()->_id]
                       ->eva_nr.c_str());

      // [M]otis [T]ime [O]ffset (1*MINUTES_A_DAY - GMT+1)
      auto const MTO = SCHEDULE_OFFSET_MINUTES - 60;
      ASSERT_EQ(1, first_route_node->_edges[1]._m._route_edge._conns.size());
      auto& lcon = first_route_node->_edges[1]._m._route_edge._conns;
      ASSERT_EQ(19 * 60 + 3 + MTO, lcon[0].d_time);
      ASSERT_EQ(19 * 60 + 58 + MTO, lcon[0].a_time);

      auto connections = get_connections(first_route_node, 19 * 60 + 3);
      ASSERT_EQ(8, static_cast<int>(connections.size()));

      auto& stations = sched_->stations;
      EXPECT_EQ("8000284",
                stations[get<1>(connections[0])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8000260",
                stations[get<1>(connections[1])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8010101",
                stations[get<1>(connections[2])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8010240",
                stations[get<1>(connections[3])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8010205",
                stations[get<1>(connections[4])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8010222",
                stations[get<1>(connections[5])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8011113",
                stations[get<1>(connections[6])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8098160",
                stations[get<1>(connections[7])->get_station()->_id]->eva_nr);
      EXPECT_EQ("8011102",
                stations[get<2>(connections[7])->get_station()->_id]->eva_nr);

      EXPECT_EQ(19 * 60 + 3 + MTO, get<0>(connections[0])->d_time);
      EXPECT_EQ(20 * 60 + MTO, get<0>(connections[1])->d_time);
      EXPECT_EQ(21 * 60 + 55 + MTO, get<0>(connections[2])->d_time);
      EXPECT_EQ(22 * 60 + 34 + MTO, get<0>(connections[3])->d_time);
      EXPECT_EQ(23 * 60 + 18 + MTO, get<0>(connections[4])->d_time);
      EXPECT_EQ(23 * 60 + 49 + MTO, get<0>(connections[5])->d_time);
      EXPECT_EQ(24 * 60 + 25 + MTO, get<0>(connections[6])->d_time);
      EXPECT_EQ(24 * 60 + 33 + MTO, get<0>(connections[7])->d_time);

      EXPECT_EQ(19 * 60 + 58 + MTO, get<0>(connections[0])->a_time);
      EXPECT_EQ(21 * 60 + 53 + MTO, get<0>(connections[1])->a_time);
      EXPECT_EQ(22 * 60 + 32 + MTO, get<0>(connections[2])->a_time);
      EXPECT_EQ(23 * 60 + 8 + MTO, get<0>(connections[3])->a_time);
      EXPECT_EQ(23 * 60 + 47 + MTO, get<0>(connections[4])->a_time);
      EXPECT_EQ(24 * 60 + 23 + MTO, get<0>(connections[5])->a_time);
      EXPECT_EQ(24 * 60 + 30 + MTO, get<0>(connections[6])->a_time);
      EXPECT_EQ(24 * 60 + 38 + MTO, get<0>(connections[7])->a_time);

      ASSERT_TRUE(std::all_of(
          begin(connections), end(connections),
          [&](std::tuple<light_connection const*, node const*,
                         node const*> const& con) {
            auto fc = std::get<0>(con)->_full_con;
            return fc->con_info->attributes.size() == 1 &&
                   fc->con_info->train_nr == 1000 &&
                   sched_->categories[fc->con_info->family]->name == "ICE";
          }));

      for (auto const& c : connections) {
        auto fc = std::get<0>(c)->_full_con;
        ASSERT_STREQ("---", fc->con_info->provider_->short_name.c_str());
        ASSERT_STREQ("DB AG", fc->con_info->provider_->long_name.c_str());
        ASSERT_STREQ("Deutsche Bahn AG",
                     fc->con_info->provider_->full_name.c_str());
      }

      auto const& tracks = sched_->tracks;
      EXPECT_EQ("6", tracks[get<0>(connections[0])->_full_con->d_platform]);
      EXPECT_EQ("", tracks[get<0>(connections[1])->_full_con->d_platform]);
      EXPECT_EQ("", tracks[get<0>(connections[2])->_full_con->d_platform]);
      EXPECT_EQ("1", tracks[get<0>(connections[3])->_full_con->d_platform]);
      EXPECT_EQ("", tracks[get<0>(connections[4])->_full_con->d_platform]);
      EXPECT_EQ("3", tracks[get<0>(connections[5])->_full_con->d_platform]);
      EXPECT_EQ("8", tracks[get<0>(connections[6])->_full_con->d_platform]);
      EXPECT_EQ("7", tracks[get<0>(connections[7])->_full_con->d_platform]);

      EXPECT_EQ("", tracks[get<0>(connections[0])->_full_con->a_platform]);
      EXPECT_EQ("", tracks[get<0>(connections[1])->_full_con->a_platform]);
      EXPECT_EQ("1", tracks[get<0>(connections[2])->_full_con->a_platform]);
      EXPECT_EQ("", tracks[get<0>(connections[3])->_full_con->a_platform]);
      EXPECT_EQ("3", tracks[get<0>(connections[4])->_full_con->a_platform]);
      EXPECT_EQ("8", tracks[get<0>(connections[5])->_full_con->a_platform]);
      EXPECT_EQ("7", tracks[get<0>(connections[6])->_full_con->a_platform]);
      EXPECT_EQ("6", tracks[get<0>(connections[7])->_full_con->a_platform]);

      auto bt = get<0>(connections[0])->_full_con->con_info->attributes[0];
      auto sn = get<0>(connections[4])->_full_con->con_info->attributes[0];
      EXPECT_STREQ("BT", bt->_code.c_str());
      EXPECT_STREQ("SN", sn->_code.c_str());
      EXPECT_STREQ("Bordbistro", bt->_str.c_str());
      EXPECT_STREQ("SnackPoint/Imbiss im Zug", sn->_str.c_str());

      EXPECT_EQ(bt, get<0>(connections[0])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(bt, get<0>(connections[1])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(bt, get<0>(connections[2])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(bt, get<0>(connections[3])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(sn, get<0>(connections[4])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(sn, get<0>(connections[5])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(sn, get<0>(connections[6])->_full_con->con_info->attributes[0]);
      EXPECT_EQ(sn, get<0>(connections[7])->_full_con->con_info->attributes[0]);

      EXPECT_EQ(2, get<1>(connections[6])->_incoming_edges.size());
      EXPECT_TRUE(std::any_of(
          begin(get<1>(connections[6])->_incoming_edges),
          end(get<1>(connections[6])->_incoming_edges), [&](edge const* e) {
            return e->type() == edge::INVALID_EDGE &&
                   e->_from == get<1>(connections[6])->get_station();
          }));
      EXPECT_TRUE(std::any_of(
          begin(get<1>(connections[6])->_incoming_edges),
          end(get<1>(connections[6])->_incoming_edges),
          [](edge const* e) { return e->type() == edge::ROUTE_EDGE; }));

      EXPECT_TRUE(std::none_of(
          begin(get<1>(connections[5])->_incoming_edges),
          end(get<1>(connections[5])->_incoming_edges),
          [](edge const* e) { return e->type() == edge::INVALID_EDGE; }));
    } else {
      auto connections = get_connections(first_route_node, 17 * 60 + 39);
      ASSERT_EQ(0, connections.size());
    }
  }
}

}  // loader
}  // motis