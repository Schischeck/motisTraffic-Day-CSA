#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"
#include "motis/loader/parsers/hrd/hrd_parser.h"
#include "motis/loader/graph_builder.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "./hrd/test_spec.h"

namespace motis {
namespace loader {

auto interval_begin = to_unix_time(2015, 10, 25);
auto interval_end = interval_begin + (2 * MINUTES_A_DAY * 60);

class graph_builder_test : public ::testing::Test {
protected:
  virtual void SetUp() {
    hrd::hrd_parser parser;

    const auto schedule_path = hrd::SCHEDULES / "multiple-ice-files";
    ASSERT_TRUE(parser.applicable(schedule_path));

    flatbuffers::FlatBufferBuilder b;
    parser.parse(schedule_path, b);
    auto serialized = GetSchedule(b.GetBufferPointer());

    sched_ = build_graph(serialized, interval_begin, interval_end);
  }

  static edge const* get_route_edge(node const* route_node) {
    auto it = std::find_if(
        begin(route_node->_edges), end(route_node->_edges),
        [](edge const& e) { return e.type() == edge::ROUTE_EDGE; });
    if (it == end(route_node->_edges)) {
      return nullptr;
    } else {
      return &(*it);
    }
  }

  static std::vector<std::tuple<light_connection const*, int, int>>
  get_connections(node const* first_route_node, time departure_time) {
    std::vector<std::tuple<light_connection const*, int, int>> cons;
    edge const* route_edge = nullptr;
    node const* route_node = first_route_node;
    while ((route_edge = get_route_edge(route_node)) != nullptr) {
      cons.emplace_back(route_edge->get_connection(departure_time),
                        route_node->get_station()->_id,
                        route_edge->_to->get_station()->_id);
      route_node = route_edge->_to;
      departure_time = std::get<0>(cons.back())->a_time;
    }
    return cons;
  }

  schedule_ptr sched_;
};

TEST_F(graph_builder_test, interval_test) {
  ASSERT_EQ(interval_begin, sched_->schedule_begin_);
  ASSERT_EQ(interval_end, sched_->schedule_end_);
}

TEST_F(graph_builder_test, eva_num) {
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

TEST_F(graph_builder_test, simple_test) {
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

TEST_F(graph_builder_test, coordinates) {
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

TEST_F(graph_builder_test, interchange_edges) {
  // TODO(felix) check interchange times
}

TEST_F(graph_builder_test, route_nodes) {
  EXPECT_EQ(2, sched_->route_index_to_first_route_node.size());

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

      ASSERT_EQ(1, first_route_node->_edges[1]._m._route_edge._conns.size());
      auto& lcon = first_route_node->_edges[1]._m._route_edge._conns;
      ASSERT_EQ(19 * 60 + 3, lcon[0].d_time);
      ASSERT_EQ(19 * 60 + 58, lcon[0].a_time);

      auto connections = get_connections(first_route_node, 19 * 60 + 3);
      ASSERT_EQ(8, static_cast<int>(connections.size()));

      for (auto const& con : connections) {
        light_connection const* lc;
        int from, to;
        std::tie(lc, from, to) = con;

        auto const& fc = lc->_full_con;
        auto const& ci = fc->con_info;

        std::cout << "[" << sched_->stations[from]->eva_nr << "] "
                  << "[" << sched_->stations[to]->eva_nr << "]\t"
                  << format_time(lc->d_time) << " - " << format_time(lc->a_time)
                  << "\t[" << std::setw(1) << sched_->tracks[fc->d_platform]
                  << "] [" << std::setw(1) << sched_->tracks[fc->a_platform]
                  << "]\t" << sched_->category_names[ci->family] << " "
                  << ci->train_nr << "\t";
        for (auto const& attr : ci->attributes) {
          std::cout << attr->_code << " [" << attr->_str << "]";
        }
        std::cout << "\n";
      }
    }
  }
}

}  // loader
}  // motis
