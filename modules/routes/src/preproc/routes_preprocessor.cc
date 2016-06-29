#include <fstream>
#include <iostream>
#include <vector>

#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include "boost/algorithm/string/predicate.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/station.h"

#include "motis/loader/loader.h"

#include "motis/routes/preproc/db/railway_finder.h"
#include "motis/routes/preproc/db/railway_graph.h"
#include "motis/routes/preproc/db/railway_graph_builder.h"
#include "motis/routes/preproc/db/railway_graph_dijkstra.h"

using namespace rapidjson;

using namespace motis;
using namespace motis::logging;
using namespace motis::routes;
using namespace motis::loader;

template <typename T>
void erase_duplicates(std::vector<T>& vec) {
  std::sort(begin(vec), end(vec));
  vec.erase(std::unique(begin(vec), end(vec)), end(vec));
}

railway_node const* find_node(railway_graph const& graph,
                              std::string const& ds100) {
  auto const it = graph.ds100_to_node_.find(ds100);
  if (it == end(graph.ds100_to_node_)) {
    return nullptr;
  }
  return it->second;
}

railway_node const* find_node(railway_graph const& graph, schedule const& sched,
                              station const* s) {
  for (auto const& pair : sched.ds100_to_station_) {
    if (pair.second != s) {
      continue;
    }

    auto node = find_node(graph, pair.first);
    if (node != nullptr) {
      return node;
    }
  }
  return nullptr;
}

std::vector<station const*> get_neighbors(schedule const& sched,
                                          station const* s) {
  std::vector<station const*> result;

  auto const& station_node = sched.station_nodes_[s->index_];
  for (auto const& route_node : station_node->get_route_nodes()) {
    for (auto const& edge : route_node->edges_) {
      if (edge.type() != edge::ROUTE_EDGE) {
        continue;
      }

      result.push_back(sched.stations_[edge.to_->get_station()->id_].get());
    }
  }
  erase_duplicates(result);
  return result;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "./routes-preprocessor <geojsons> <schedule (default "
                 "rohdaten)> <schedule_begin>"
              << std::endl;
    return 0;
  }

  railway_graph graph;
  railway_graph_builder graph_builder(graph);
  graph_builder.build_graph(argv[1]);

  loader_options opt(argv[2], argv[3], 2, true, false, false, false);
  auto sched = load_schedule(opt);

  auto it = sched->ds100_to_station_.find("FD");
  if (it == end(sched->ds100_to_station_)) {
    std::cout << "station not found" << std::endl;
    return 1;
  }

  auto from_railway_node = find_node(graph, "FD");
  if (from_railway_node == nullptr) {
    std::cout << "station not found in DB NETZ" << std::endl;
    return 1;
  }

  auto neighbors = get_neighbors(*sched, it->second);

  std::vector<railway_node const*> goal_nodes;
  std::vector<size_t> goal_idx;
  for (auto const& n : neighbors) {
    auto node = find_node(graph, *sched, n);

    if (node == nullptr) {
      std::cout << n->name_ << " not found in DB NETZ" << std::endl;
      break;
    }

    std::cout << node->idx_ << " " << n->name_ << std::endl;

    // if(n->name_ != "Weinheim(Bergstr)") {
    //   continue;
    // }

    goal_nodes.push_back(node);
    goal_idx.push_back(node->idx_);
  }

  railway_graph_dijkstra d(graph, from_railway_node->idx_, goal_idx);
  d.run();

  FILE* fp = std::fopen("geo.json", "w");
  char writeBuffer[65536];

  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::Writer<rapidjson::FileWriteStream> w(os);

  w.StartObject();
  w.String("type").String("FeatureCollection");
  w.String("features").StartArray();

  for (auto const& goal : goal_idx) {
    w.StartObject();
    w.String("type").String("Feature");
    w.String("properties").StartObject().EndObject();
    w.String("geometry").StartObject();
    w.String("type").String("LineString");
    w.String("coordinates").StartArray();

    for (auto const& link : d.get_links(goal)) {
      std::cout << link->id_ << " " << link->polyline_.size() << std::endl;

      for(auto const& coord : link->polyline_) {
        w.StartArray();
        w.Double(coord.lng_);
        w.Double(coord.lat_);
        w.EndArray();
      }
    }

    w.EndArray();
    w.EndObject();
    w.EndObject();
  }

  w.EndArray();
  w.EndObject();
}
