#include "catch/catch.hpp"

#include "motis/loader/loader.h"
#include "motis/core/schedule/schedule.h"
#include "motis/railviz/timetable_retriever.h"

TEST_CASE("route-nodes have only one route-edge", "[railviz]") {
  std::cout << "load sched..." << std::endl;
  auto schedule =
      motis::load_schedule("/home/lars/uni/bp/demo-clean/test");

  for (auto const& station_node : schedule->station_nodes) {
    for (auto const& route_node : station_node->get_route_nodes()) {
      int route_edges = 0;
      int other_edges = 0;
      for (auto const& edge : route_node->_edges) {
        if (edge.type() != motis::edge::ROUTE_EDGE) {
          ++other_edges;
          continue;
        }
          ++route_edges;
      }
      REQUIRE(route_edges <= 1);
    }
  }
}

TEST_CASE("train-routes are generated correctly", "[railviz]") {
  std::cout << "load sched..." << std::endl;
  auto schedule =
      motis::load_schedule("/home/lars/uni/bp/demo-clean/test");
  motis::railviz::timetable_retriever t_r;
  t_r.init( *schedule.get() );

  int id = 0;
  std::cout << "Looking for Traisa, Mühltal" << std::endl;
  for( auto const& stationp : schedule.get()->stations ) {
    std::string name = stationp.get()->name;
    if(name == "Traisa, Mühltal") {
      std::cout << "found(" << stationp.get()->index << ")" << std::endl;
      id = stationp.get()->index;
    }
  }

  if( id > 0 )
  {
    const motis::station_node* n = schedule.get()->station_nodes[id].get();
    std::cout << "gen route" << std::endl;
    std::vector<motis::railviz::route> routes = t_r.get_routes_on_time(46055, 2726);
    std::cout << "finish" << std::endl;
    std::cout << "found " << routes.size() << " routes" << std::endl;
    for( motis::railviz::route const& r : routes ) {
      for( int i = 0; i < r.size()-1; ++i ) {
        const motis::station_node* now = r.at(i).first;
        const motis::station_node* next = r.at( i+1 ).first;
        std::cout << schedule.get()->stations[now->_id].get()->name << r.at(i).second->d_time;
        if( next != NULL ) {
          std::cout << "->" << r.at(i).second->a_time << schedule.get()->stations[next->_id].get()->name << std::endl;
        }
        std::cout << std::endl;
      }
    }
  } else
  {
    std::cout << "error" << std::endl;
  }
}
