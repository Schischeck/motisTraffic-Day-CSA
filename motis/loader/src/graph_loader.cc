#include "motis/loader/graph_loader.h"

#include <cstdio>
#include <cmath>
#include <set>
#include <sstream>
#include <string>

#include "boost/lexical_cast.hpp"

#include "motis/core/schedule/station.h"
#include "motis/core/schedule/date_manager.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/time.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/waiting_time_rules.h"
#include "motis/loader/bitset_manager.h"
#include "motis/loader/files.h"

#ifndef M_PI
#define M_PI (3.14159265359)
#endif

#define d2r (M_PI / 180.0)

using namespace std;

namespace motis {

int graph_loader::get_price_per_km(int clasz) {
  switch (clasz) {
    case MOTIS_ICE:
      return 22;

    case MOTIS_N:
    case MOTIS_IC:
    case MOTIS_X:
      return 18;

    case MOTIS_RE:
    case MOTIS_RB:
    case MOTIS_S:
    case MOTIS_U:
    case MOTIS_STR:
    case MOTIS_BUS:
      return 15;

    default:
      return 0;
  }
}

class input_connection : public connection, public connection_info {
public:
  input_connection()
      : traffic_day_index(0), d_time(INVALID_TIME), a_time(INVALID_TIME) {}

  bitset_index traffic_day_index;
  int d_time, a_time;
};

template <typename T>
std::vector<T> join(std::vector<std::vector<T>> vecs) {
  if (vecs.empty()) {
    return {};
  } else if (vecs.size() == 1) {
    return vecs[0];
  } else {
    std::vector<T> ret = std::move(*std::begin(vecs));
    std::for_each(std::next(std::begin(vecs)), std::end(vecs),
                  [&ret](std::vector<T> const& vec) {
                    ret.insert(std::end(ret), std::begin(vec), std::end(vec));
                  });
    return ret;
  }
}

graph_loader::graph_loader(const std::string& prefix) : _prefix(prefix) {}

std::istream& operator>>(std::istream& in, station& station) {
  char c;
  in >> station.index >> c;
  std::string s;
  getline(in, s);
  int i1 = s.find('|');
  int i2 = s.find('|', i1 + 1);
  station.eva_nr = boost::lexical_cast<int>(s.substr(0, i1));
  station.name = s.substr(i1 + 1, i2 - i1 - 1);

  int i3 = s.rfind('|');
  s = s.substr(i3 + 1, s.size() - i3 - 1);

  unsigned time_difference;
  unsigned k_m_info;
  unsigned footpaths_out;
  unsigned footpaths_in;
  istringstream iss(s);
  iss >> time_difference >> station.length >> station.width >> k_m_info >>
      station.us_hoch >> station.us_nieder >> footpaths_out >> footpaths_in;

  return in;
}

int graph_loader::load_stations(std::vector<station_ptr>& stations,
                                std::vector<station_node_ptr>& station_nodes) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + STATIONS_FILE);

  int one;
  in >> one;
  assert(one == 1);
  std::string dummy;
  std::getline(in, dummy);

  int node_id = 0;

  station dummy1;
  dummy1.eva_nr = -1;
  dummy1.index = node_id;
  dummy1.name = "DUMMY";

  stations.emplace_back(new station(dummy1), deleter<station>(true));
  station_nodes.emplace_back(new station_node(node_id++),
                             deleter<station_node>(true));

  int i = 1;
  while (!in.eof()) {
    if (in.peek() == '\n' || in.eof()) {
      assert(in.eof());
      break;
    }

    stations.emplace_back(new station(), deleter<station>(true));
    in >> *stations[i];
    if (stations[i]->index != i) {
      std::cout << i << " " << stations[i]->index << "\n";
    }
    assert(stations[i]->index == i);

    station_nodes.emplace_back(new station_node(node_id++),
                               deleter<station_node>(true));
    assert(station_nodes[i]->_id == static_cast<unsigned>(i));

    ++i;
  }

  station dummy2 = dummy1;
  dummy2.eva_nr = -2;
  dummy2.index = node_id;
  dummy2.name = "DUMMY";

  stations.emplace_back(new station(dummy2), deleter<station>(true));
  station_nodes.emplace_back(new station_node(node_id++),
                             deleter<station_node>(true));

  return node_id;
}

void graph_loader::load_bitfields(bitset_manager& bm) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + BITFIELDS_FILE);
  bm = bitset_manager(in);
}

int graph_loader::load_routes(
    int node_id, std::vector<station_ptr>& stations,
    std::vector<station_node_ptr> const& station_nodes,
    const std::map<int, int>& class_mapping,
    std::vector<std::unique_ptr<connection>>& full_connections,
    std::vector<std::unique_ptr<connection_info>>& connection_infos,
    std::vector<node*>& route_index_to_first_route_node) {
  bitset_manager bm;
  load_bitfields(bm);

  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + ROUTES_FILE);

  uint32_t con_info_id = 0;
  std::map<connection_info, uint32_t> con_infos;

  unsigned index;
  while (!in.eof()) {
    if (in.peek() == '\n' || in.eof()) {
      assert(in.eof());
      break;
    }

    char c;
    unsigned n_stations, n_trains;
    in >> c;
    assert(c == 'r');
    in >> index;
    in >> n_stations >> n_trains;
    assert(n_stations > 1);
    assert(n_trains > 0);

    vector<bool> skip_arrival(n_stations), skip_departure(n_stations);
    vector<unsigned> locations(n_stations), family(n_stations - 1);
    for (unsigned i = 0; i < n_stations; ++i) {
      if (i > 0) {
        in >> c;
        assert(c == ',');
      }

      in >> c;
      assert(c == 'l');
      in >> locations[i];

      if (i > 0) {
        in >> c;
        assert(c == 'a');
        int temp;
        in >> temp;
        skip_arrival[i] = (temp == 1);
      }

      if (i < n_stations - 1) {
        in >> c;
        assert(c == 'd');
        int temp;
        in >> temp >> family[i];
        skip_departure[i] = (temp == 1);
      }
    }

    // first layer: section between stations: (n_stations - 1) sections
    // second layer: elementary connections (not expanded bitfields)
    vector<vector<input_connection>> input_connections(n_stations - 1);
    for (unsigned train_i = 0; train_i < n_trains; ++train_i) {
      in >> c;
      assert(c == 't');
      int index2, service;
      in >> index2;
      in >> c;
      assert(c == 's');
      in >> service;

      // associate the route index with this service index
      for (unsigned station_i = 0; station_i < n_stations; ++station_i) {
        if (station_i > 0) {
          in >> c;
          assert(c == 'a');
          in >> input_connections[station_i - 1][train_i].a_time >>
              input_connections[station_i - 1][train_i].a_platform;
          input_connections[station_i - 1][train_i].service = service;
        }

        if (station_i < n_stations - 1) {
          if (train_i == 0) input_connections[station_i].resize(n_trains);

          station& from_station = *stations[locations[station_i]];
          station& to_station = *stations[locations[station_i + 1]];

          int fam = family[station_i];
          std::map<int, int>::const_iterator it;
          if (fam != -1) {
            it = class_mapping.find(fam);

            assert(it != class_mapping.end());
            assert(it->second < 10);

            ++from_station.dep_class_events[it->second];
            ++to_station.arr_class_events[it->second];

            input_connections[station_i][train_i].clasz = it->second;
          }

          input_connections[station_i][train_i].family = fam;
          input_connections[station_i][train_i].price =
              get_distance(from_station, to_station) *
              get_price_per_km(input_connections[station_i][train_i].clasz);

          in >> c;
          assert(c == 'd');
          int tdi;
          int transfer_class_dummy;
          int linienzuordnung_dummy;
          in >> input_connections[station_i][train_i].d_time >>
              input_connections[station_i][train_i].d_platform >> tdi >>
              transfer_class_dummy >> linienzuordnung_dummy;
          input_connections[station_i][train_i].traffic_day_index =
              bm.get_new_index(tdi);

          int n;
          in >> n;
          input_connections[station_i][train_i].attributes.resize(n);
          for (int k = 0; k < n; ++k) {
            in >> input_connections[station_i][train_i].attributes[k];
          }

          in >> input_connections[station_i][train_i].train_nr;

          in >> c;
          assert(c == '%');

          std::string eva_nr_dummy;
          getline(in, eva_nr_dummy, '|');
          getline(in, input_connections[station_i][train_i].line_identifier,
                  '|');

          in >> c;
          assert(c == ',');
        }
      }
    }

    // expand connections to have absolute times
    //(remove bitfield dependency)
    // first layer: section between stations: (n_stations - 1) sections
    // second layer: elementary trains (not expanded bitfields)
    // third layer: expanded connections (expanded, absolute times)
    vector<vector<vector<light_connection>>> expanded_connections(n_stations -
                                                                  1);
    time prev_arr_time = 0;
    for (unsigned train_i = 0; train_i < n_trains; ++train_i) {
      for (unsigned con_i = 0; con_i < input_connections.size(); ++con_i) {
        if (expanded_connections[con_i].size() != n_trains)
          expanded_connections[con_i].resize(n_trains);

        connection_info con_info(input_connections[con_i][train_i]);
        auto it = con_infos.find(con_info);
        if (it == std::end(con_infos)) {
          std::tie(it, std::ignore) = con_infos.insert(
              std::make_pair(std::move(con_info), con_info_id++));
        }

        connection* full_con =
            new connection(input_connections[con_i][train_i]);
        full_con->con_info_id = it->second;
        full_connections.emplace_back(full_con);

        expanded_connections[con_i][train_i] =
            expand_bitfields(full_con, input_connections[con_i][train_i], bm);
      }
    }

    // build graph for the route
    node* prev_route_node = nullptr;
    for (unsigned station_i = 0; station_i < n_stations; ++station_i) {
      station_node* station = station_nodes[locations[station_i]].get();

      // build the new route node
      node* route_node = new node(station, node_id++);
      route_node->_route = index;

      if (station_i == 0) {
        assert(route_index_to_first_route_node.size() == index);
        route_index_to_first_route_node.push_back(route_node);
      }

      if (station_i > 0) {
        auto joined_connections = join(expanded_connections[station_i - 1]);
        std::sort(joined_connections.begin(), joined_connections.end());
        if (station_i > 1 && (joined_connections.size() == 0 ||
                              joined_connections[0].d_time < prev_arr_time)) {
          std::cout << "ignore rest of route " << index << std::endl;
          delete route_node;
          break;
        }
        prev_arr_time = joined_connections[0].a_time;

        if (!skip_arrival[station_i]) {
          route_node->_edges.emplace_back(make_foot_edge(
              station_nodes[locations[station_i]].get(),
              stations[locations[station_i]]->get_transfer_time(), true));
        }

        prev_route_node->_edges.emplace_back(
            make_route_edge(route_node, joined_connections));
      }

      if (station_i < n_stations - 1 && !skip_departure[station_i]) {
        station->_edges.push_back(make_foot_edge(route_node));
      } else {
        station->_edges.push_back(make_invalid_edge(route_node));
      }

      prev_route_node = route_node;
    }

    std::string dummy;
    getline(in, dummy);
  }

  // reverse mapping of connection infos
  std::map<uint32_t, connection_info*> con_info_id2con_info_ptr;
  for (auto const& con_info : con_infos) {
    connection_info* info = new connection_info(con_info.first);
    con_info_id2con_info_ptr[con_info.second] = info;
    connection_infos.emplace_back(info);
  }

  // initialize pointers from the full connections to connection infos
  for (auto& full_con : full_connections) {
    full_con->con_info = con_info_id2con_info_ptr[full_con->con_info_id];
  }

  return node_id;
}

void link_predecessor(node* node, edge& edge) {
  edge._from = node;
  edge.get_destination()->_incoming_edges.push_back(&edge);
}

void graph_loader::assign_predecessors(
    std::vector<station_node_ptr>& station_nodes) {
  for (auto& station_node : station_nodes) {
    for (auto& station_edge : station_node->_edges) {
      link_predecessor(station_node.get(), station_edge);

      node* node = station_edge.get_destination();
      for (auto& edge : node->_edges) {
        link_predecessor(node, edge);
      }
    }
  }
}

std::vector<light_connection> graph_loader::expand_bitfields(
    connection const* full_con, input_connection const& con,
    bitset_manager const& bm) {
  std::vector<light_connection> ret;

  for (auto const& day : bm.get_active_day_indices(con.traffic_day_index)) {
    time d_time = (con.d_time == INVALID_TIME)
                      ? INVALID_TIME
                      : day * MINUTES_A_DAY + con.d_time,
         a_time = (con.a_time == INVALID_TIME)
                      ? INVALID_TIME
                      : day * MINUTES_A_DAY + con.a_time;

    if (d_time != INVALID_TIME && a_time != INVALID_TIME && d_time > a_time) {
      a_time += MINUTES_A_DAY;
    }

    ret.emplace_back(d_time, a_time, full_con);
  }

  return ret;
}

int graph_loader::load_foot_paths(
    int node_id, std::vector<station_node_ptr> const& station_nodes) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + FOOTPATHS_FILE);

  while (!in.eof()) {
    if (in.peek() == '\n' || in.eof()) {
      assert(in.eof());
      break;
    }

    int from, to, duration;
    in >> from >> to >> duration;

    node_id = station_nodes[from]->add_foot_edge(
        node_id, make_foot_edge(station_nodes[to].get(), duration));

    std::string dummy;
    getline(in, dummy);
  }

  return node_id;
}

void graph_loader::load_attributes(std::map<int, attribute>& attributes_map) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + ATTRIBUTES_FILE);

  int index;
  while (!in.eof()) {
    if (in.peek() == '\n' || in.eof()) {
      assert(in.eof());
      break;
    }

    char c;
    in >> index >> c;

    std::string s;
    getline(in, s);
    std::size_t p1 = s.find("|");
    assert(p1 != std::string::npos);

    std::size_t p2 = s.find("|", p1 + 1);
    assert(p2 != std::string::npos);

    attributes_map[index]._str = s.substr(0, p1);
    attributes_map[index]._code = s.substr(p1 + 1, p2 - p1 - 1);
  }
}

void graph_loader::load_classes(std::map<std::string, int>& map) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + CLASSES_FILE);

  int n_classes;
  in >> n_classes;

  int i = 0;
  while (!in.eof() && in.peek() != EOF && ++i <= n_classes) {
    int index;
    in >> index;

    int n_categories;
    in >> n_categories;

    std::string cat;
    for (int j = 0; j < n_categories; ++j) {
      in >> cat;
      map[cat] = index;
    }
  }
}

void graph_loader::load_category_names(
    std::vector<std::string>& category_names) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + CATEGORIES_FILE);

  // category indices are 1-based.
  // push dummy category name.
  category_names.push_back("DUMMY");

  int index, category;
  while (!in.eof()) {
    if (in.peek() == '\n' || in.eof()) {
      assert(in.eof());
      break;
    }

    in >> index >> category;

    std::string s;
    getline(in, s);

    std::size_t p1 = s.find("|");
    assert(p1 != std::string::npos);

    std::size_t p2 = s.find("|", p1 + 1);
    assert(p2 != std::string::npos);

    category_names.push_back(s.substr(p1 + 1, p2 - p1 - 1));
  }
}

void graph_loader::load_tracks(std::map<int, std::string>& track_map) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + TRACK_FILE);

  int n_tracks;
  in >> n_tracks;

  std::string buf;
  getline(in, buf);

  int index;
  while (!in.eof()) {
    if (in.peek() == '\n' || in.eof()) {
      assert(in.eof());
      break;
    }

    char c;
    in >> index >> c;
    assert(c == '.');

    std::string track = track_map[index];
    getline(in, track);

    std::size_t cr_pos = track.find("\r");
    track = (cr_pos == std::string::npos) ? track : track.substr(0, cr_pos);
    track_map[index] = track;
  }
}

void graph_loader::load_dates(date_manager& dm) {
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + DATES_FILE);

  int n;
  in >> n;
  vector<date_manager::date> dates(n);
  char c;
  int ind;
  for (int i = 0; i < n; ++i) {
    in >> ind >> c >> dates[i].day >> c >> dates[i].month >> c >>
        dates[i].year >> c;
  }

  dm.load(dates);
}

void graph_loader::load_waiting_time_rules(
    std::vector<std::string> const& category_names, waiting_time_rules& rules) {
  rules.default_group = 6;

  // clang-format off
  std::vector<std::vector<std::string>> categories{
  { "EC", "ICE", "IC", "Tha", "CIS", "RHT", "RHI" },
  { "EN", "NZ", "D", "CNL", "TLG", "DNZ" },
  { "IRE", "RE", "RB" },
  { "S", "s" },
  { "DPE", "DPN", "R", "IRX", "X", "E", "SCH", "BSV", "RT", "FB", "LX", "REX" },
  { } /* default group */
  };
  // clang-format on

  // clang-format off
  std::vector<int> waiting_times{
  3, 0, 0, 0, 0, 0,
  10, 10, 0, 0, 5, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  5, 5, 0, 5, 5, 0,
  0, 0, 0, 0, 0, 0 /* default group */
  };
  // clang-format on

  // category/group numbers are 1-based.
  // initializes all values to false.
  int number_of_groups = categories.size();
  rules._waits_for_other_trains.resize(number_of_groups + 1);
  rules._other_trains_wait_for.resize(number_of_groups + 1);
  rules._waiting_time_matrix = flat_matrix<duration>(number_of_groups + 1);

  for (int i = 0; i < number_of_groups; i++) {
    int group = i + 1;  // groups are one-based
    for (auto const& category_name : categories[i]) {
      rules._category_map[category_name] = group;
    }
  }

  for (int i = 0; i < number_of_groups * number_of_groups; i++) {
    int connecting_cat = i / number_of_groups + 1;
    int feeder_cat = i % number_of_groups + 1;
    int waiting_time = waiting_times[i];

    rules._waiting_time_matrix[connecting_cat][feeder_cat] = waiting_time;

    if (waiting_time > 0) {
      rules._waits_for_other_trains[connecting_cat] = true;
      rules._other_trains_wait_for[feeder_cat] = true;
    }
  }

  rules._family_to_wtr_category.resize(category_names.size());
  for (size_t i = 0; i < category_names.size(); i++) {
    rules._family_to_wtr_category[i] =
        rules.waiting_time_category(category_names[i]);
  }
}

double graph_loader::get_distance(const station& s1, const station& s2) {
  double lat1 = s1.width, long1 = s1.length;
  double lat2 = s2.width, long2 = s2.length;
  double dlong = (long2 - long1) * d2r;
  double dlat = (lat2 - lat1) * d2r;
  double a = pow(sin(dlat / 2.0), 2) +
             cos(lat1 * d2r) * cos(lat2 * d2r) * pow(sin(dlong / 2.0), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double d = 6367 * c;
  return d;
}

}  // namespace motis
