#include "motis/loader/loader.h"

#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"

#include "motis/loader/bitset_manager.h"
#include "motis/loader/graph_loader.h"
#include "motis/loader/deserializer.h"
#include "motis/loader/files.h"

namespace motis {

std::map<int, int> build_category_class_map(
    std::vector<std::string> const& category_names,
    std::map<std::string, int> classes) {
  std::map<int, int> category_class_map;

  for (unsigned i = 0; i < category_names.size(); ++i) {
    auto it = classes.find(category_names[i]);
    if (it == end(classes)) {
      category_class_map[i] = 9;
    } else {
      category_class_map[i] = it->second;
    }
  }

  return category_class_map;
}

schedule_ptr load_schedule(std::string const& prefix) {
  if (boost::filesystem::exists(prefix + SCHEDULE_FILE)) {
    return load_binary_schedule(prefix);
  } else {
    return load_text_schedule(prefix);
  }
}

schedule_ptr load_text_schedule(std::string const& prefix) {
  std::unique_ptr<text_schedule> s(new text_schedule());

  graph_loader loader(prefix);
  loader.load_dates(s->date_mgr);
  int node_id = loader.load_stations(s->stations, s->station_nodes);
  loader.load_classes(s->classes);
  loader.load_category_names(s->category_names);
  node_id = loader.load_routes(
      node_id, s->stations, s->station_nodes,
      build_category_class_map(s->category_names, s->classes), s->full_connections,
      s->connection_infos, s->route_index_to_first_route_node);
  node_id = loader.load_foot_paths(node_id, s->station_nodes);
  loader.load_tracks(s->tracks);
  loader.load_attributes(s->attributes);
  loader.assign_predecessors(s->station_nodes);
  loader.load_waiting_time_rules(s->category_names, s->waiting_time_rules_);

  s->node_count = node_id;

  s->lower_bounds = constant_graph(s->station_nodes);

  for (auto const& station : s->stations) {
    s->eva_to_station[boost::lexical_cast<int>(station->eva_nr)] = station.get();
  }

  return schedule_ptr(s.release());
}

schedule_ptr load_binary_schedule(std::string const& prefix) {
  std::unique_ptr<binary_schedule> s(new binary_schedule());

  graph_loader loader(prefix);
  loader.load_dates(s->date_mgr);
  loader.load_classes(s->classes);
  loader.load_category_names(s->category_names);
  loader.load_tracks(s->tracks);
  loader.load_attributes(s->attributes);
  loader.load_waiting_time_rules(s->category_names, s->waiting_time_rules_);

  deserializer deserializer(prefix);
  std::tie(s->node_count, s->raw_memory) =
      deserializer.load_graph(s->stations, s->station_nodes);

  s->lower_bounds = constant_graph(s->station_nodes);

  for (auto const& station : s->stations) {
    s->eva_to_station[boost::lexical_cast<int>(station->eva_nr)] = station.get();
  }

  return schedule_ptr(s.release());
}

}  // namespace motis
