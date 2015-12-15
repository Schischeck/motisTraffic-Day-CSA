#include "motis/loader/graph_builder.h"

#include <set>
#include <cassert>
#include <functional>
#include <algorithm>
#include <unordered_set>

#include "parser/cstr.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"
#include "motis/core/common/hash_helper.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/price.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/timezone.h"
#include "motis/core/schedule/print.h"

#include "motis/loader/wzr_loader.h"
#include "motis/loader/util.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/classes.h"
#include "motis/loader/timezone_util.h"
#include "motis/loader/duplicate_checker.h"

using namespace motis::logging;
using namespace flatbuffers;

namespace motis {
namespace loader {

using route_lcs = std::vector<std::vector<light_connection>>;

struct route_info {
  route_info() = default;
  route_info(node* rn, int edge_idx)
      : route_node(rn), outgoing_route_edge_index(edge_idx) {
    assert(rn == nullptr || rn->is_route_node());
  }

  edge* get_route_edge() {
    if (outgoing_route_edge_index == -1) {
      return nullptr;
    }

    assert(outgoing_route_edge_index >= 0);
    assert(static_cast<unsigned>(outgoing_route_edge_index) <
           route_node->_edges.size());
    assert(route_node->_edges[outgoing_route_edge_index].type() ==
           edge::ROUTE_EDGE);
    return &route_node->_edges[outgoing_route_edge_index];
  }

  node* route_node;
  int outgoing_route_edge_index;
};
typedef std::vector<route_info> route;

class graph_builder {
public:
  graph_builder(schedule& sched, Interval const* schedule_interval, time_t from,
                time_t to, bool apply_rules)
      : next_route_index_(-1),
        next_node_id_(-1),
        sched_(sched),
        first_day_((from - schedule_interval->from()) / (MINUTES_A_DAY * 60)),
        last_day_((to - schedule_interval->from()) / (MINUTES_A_DAY * 60) - 1),
        apply_rules_(apply_rules) {
    connections_.set_empty_key(nullptr);
    con_infos_.set_empty_key(nullptr);
    bitfields_.set_empty_key(nullptr);
    stations_.set_empty_key(nullptr);
    duplicate_count_ = 0;
  }

  void add_stations(Vector<Offset<Station>> const* stations) {
    // Add dummy source station.
    auto dummy_source =
        make_unique<station>(0, 0.0, 0.0, 0, "-1", "DUMMY", nullptr);
    sched_.eva_to_station.insert(
        std::make_pair(dummy_source->eva_nr, dummy_source.get()));
    sched_.stations.emplace_back(std::move(dummy_source));
    sched_.station_nodes.emplace_back(make_unique<station_node>(0));

    // Add dummy target stations.
    auto dummy_target =
        make_unique<station>(1, 0.0, 0.0, 0, "-2", "DUMMY", nullptr);
    sched_.eva_to_station.insert(
        std::make_pair(dummy_target->eva_nr, dummy_target.get()));
    sched_.stations.emplace_back(std::move(dummy_target));
    sched_.station_nodes.emplace_back(make_unique<station_node>(1));

    // Add schedule stations.
    for (unsigned i = 0; i < stations->size(); ++i) {
      auto const& input_station = stations->Get(i);
      auto const station_index = i + 2;

      // Create station node.
      auto node_ptr = make_unique<station_node>(station_index);
      stations_[input_station] = node_ptr.get();
      sched_.station_nodes.emplace_back(std::move(node_ptr));

      // Create station object.
      auto s = make_unique<station>();
      s->index = station_index;
      s->name = input_station->name()->str();
      s->width = input_station->lat();
      s->length = input_station->lng();
      s->eva_nr = input_station->id()->str();
      s->transfer_time = input_station->interchange_time();
      s->timez = input_station->timezone()
                     ? get_or_create_timezone(input_station->timezone())
                     : nullptr;
      sched_.eva_to_station.insert(
          std::make_pair(input_station->id()->str(), s.get()));
      sched_.stations.emplace_back(std::move(s));

      // Store DS100.
      if (input_station->external_ids()) {
        for (auto const& ds100 : *input_station->external_ids()) {
          sched_.ds100_to_station.insert(std::make_pair(ds100->str(), s.get()));
        }
      }
    }

    // First regular node id:
    // first id after station node ids
    next_node_id_ = sched_.stations.size();
  }

  timezone const* get_or_create_timezone(Timezone const* input_timez) {
    return get_or_create(timezones_, input_timez, [&]() {
      auto const tz =
          input_timez->season()
              ? create_timezone(
                    input_timez->general_offset(),
                    input_timez->season()->offset(),  //
                    first_day_, last_day_,
                    input_timez->season()->day_idx_first_day(),
                    input_timez->season()->day_idx_last_day(),
                    input_timez->season()->minutes_after_midnight_first_day(),
                    input_timez->season()->minutes_after_midnight_last_day())
              : timezone(input_timez->general_offset());
      sched_.timezones.emplace_back(new timezone(tz));
      return sched_.timezones.back().get();
    });
  }

  void add_services(Vector<Offset<Service>> const* services) {
    std::vector<Service const*> sorted(services->size());
    std::copy(std::begin(*services), std::end(*services), begin(sorted));
    std::sort(begin(sorted), end(sorted),
              [](Service const* lhs, Service const* rhs) {
                return lhs->route() < rhs->route();
              });

    auto it = begin(sorted);
    while (it != end(sorted)) {
      std::vector<Service const*> route_services;
      auto route = (*it)->route();
      do {
        if (!apply_rules_ || !(*it)->rule_participant()) {
          route_services.push_back(*it);
        }
        ++it;
      } while (it != end(sorted) && route == (*it)->route());
      add_route_services(route_services);
    }
  }

  void add_route_services(std::vector<Service const*> const& services) {
    if (services.empty()) {
      return;
    }

    std::vector<route_lcs> alt_routes;
    for (auto const& s : services) {
      auto const traffic_days = get_or_create_bitfield(s->traffic_days());
      auto const first_day_offset =
          s->times()->Get(s->times()->size() - 2) / 1440;
      auto const first_day = std::max(0, first_day_ - first_day_offset);

      for (int day = first_day; day <= last_day_; ++day) {
        if (!traffic_days.test(day)) {
          continue;
        }

        time prev_arr = 0;
        bool adjusted = false;
        std::vector<light_connection> lcons;
        for (unsigned section_idx = 0; section_idx < s->sections()->size();
             ++section_idx) {
          lcons.push_back(
              section_to_connection(s, day, section_idx, prev_arr, adjusted));
          prev_arr = lcons.back().a_time;
        }

        add_to_routes(alt_routes, lcons);
      }
    }

    for (auto const& route : alt_routes) {
      if (route.empty() || route[0].empty()) {
        continue;
      }

      auto r = create_route(services[0]->route());
      for (unsigned section_idx = 0; section_idx < route.size();
           ++section_idx) {
        if (section_idx != 0) {
          verify(route[section_idx].size() == route[section_idx - 1].size(),
                 "same number of connections on each route edge");
        }
        (*r)[section_idx].get_route_edge()->_m._route_edge._conns.set(
            begin(route[section_idx]), end(route[section_idx]));
      }
    }
  }

  static int get_index(
      std::vector<std::vector<light_connection>> const& alt_route,
      std::vector<light_connection> const& sections) {
    assert(!sections.empty());
    assert(!alt_route.empty());

    if (alt_route[0].empty()) {
      return 0;
    }

    int index = -1;
    for (unsigned section_idx = 0; section_idx < sections.size();
         ++section_idx) {
      auto const& route_section = alt_route[section_idx];
      auto const& lc = sections[section_idx];
      if (index == -1) {
        index = std::distance(
            begin(route_section),
            std::lower_bound(begin(route_section), end(route_section),
                             sections[section_idx]));
      } else {
        // Check if departures stay sorted.
        bool earlier_eq_dep =
            index > 0 && lc.d_time <= route_section[index - 1].d_time;
        bool later_eq_dep =
            static_cast<unsigned>(index) < route_section.size() &&
            lc.d_time >= route_section[index].d_time;

        // Check if arrivals stay sorted.
        bool earlier_eq_arr =
            index > 0 && lc.a_time <= route_section[index - 1].a_time;
        bool later_eq_arr =
            static_cast<unsigned>(index) < route_section.size() &&
            lc.a_time >= route_section[index].a_time;

        if (earlier_eq_dep || later_eq_dep || earlier_eq_arr || later_eq_arr) {
          return -1;
        }
      }
    }
    return index;
  }

  void add_to_route(std::vector<std::vector<light_connection>>& route,
                    std::vector<light_connection> const& sections, int index) {
    for (unsigned section_idx = 0; section_idx < sections.size();
         ++section_idx) {
      route[section_idx].insert(std::next(begin(route[section_idx]), index),
                                sections[section_idx]);
    }
  }

  void add_to_routes(
      std::vector<std::vector<std::vector<light_connection>>>& alt_routes,
      std::vector<light_connection> const& sections) {
    for (auto& alt_route : alt_routes) {
      int index = get_index(alt_route, sections);
      if (index == -1) {
        continue;
      }

      add_to_route(alt_route, sections, index);
      return;
    }

    alt_routes.push_back(
        std::vector<std::vector<light_connection>>(sections.size()));
    add_to_route(alt_routes.back(), sections, 0);
  }

  light_connection section_to_connection(Service const* s, int day,
                                         unsigned section_idx, time prev_arr,
                                         bool& adjusted) {
    auto& from = *sched_.stations.at(
        stations_[s->route()->stations()->Get(section_idx)]->_id);
    auto& to = *sched_.stations.at(
        stations_[s->route()->stations()->Get(section_idx + 1)]->_id);

    auto dep_platforms = s->platforms()
                             ? s->platforms()->Get(section_idx)->dep_platforms()
                             : nullptr;
    auto arr_platforms =
        s->platforms() ? s->platforms()->Get(section_idx + 1)->arr_platforms()
                       : nullptr;

    auto section = s->sections()->Get(section_idx);
    auto dep_time = s->times()->Get(section_idx * 2 + 1);
    auto arr_time = s->times()->Get(section_idx * 2 + 2);

    // Day indices for shifted bitfields (platforms, attributes)
    int dep_day_index = day + (dep_time / MINUTES_A_DAY);
    int arr_day_index = day + (arr_time / MINUTES_A_DAY);

    // Build connection info.
    con_info_.line_identifier =
        section->line_id() ? section->line_id()->str() : "";
    con_info_.train_nr = section->train_nr();
    con_info_.family = get_or_create_category_index(section->category());
    con_info_.dir_ = get_or_create_direction(section->direction());
    con_info_.provider_ = get_or_create_provider(section->provider());
    read_attributes(dep_day_index, section->attributes(), con_info_.attributes);

    // Build full connection.
    con_.con_info = set_get_or_create(con_infos_, &con_info_, [&]() {
      sched_.connection_infos.emplace_back(
          make_unique<connection_info>(con_info_));
      return sched_.connection_infos.back().get();
    });

    con_.d_platform = get_or_create_platform(dep_day_index, dep_platforms);
    con_.a_platform = get_or_create_platform(arr_day_index, arr_platforms);

    auto clasz_it = sched_.classes.find(section->category()->name()->str());
    con_.clasz = (clasz_it == end(sched_.classes)) ? 9 : clasz_it->second;
    con_.price = get_distance(from, to) * get_price_per_km(con_.clasz);

    // Build light connection.
    time dep_motis_time, arr_motis_time;
    std::tie(dep_motis_time, arr_motis_time) =
        get_event_times(day - first_day_, prev_arr, dep_time, arr_time,
                        from.timez, to.timez, adjusted);

    // Count events.
    ++from.dep_class_events[con_.clasz];
    ++to.arr_class_events[con_.clasz];

    return light_connection(
        dep_motis_time, arr_motis_time,
        set_get_or_create(connections_, &con_, [&]() {
          sched_.full_connections.emplace_back(make_unique<connection>(con_));
          return sched_.full_connections.back().get();
        }));
  }

  void add_footpaths(Vector<Offset<Footpath>> const* footpaths) {
    for (auto const& footpath : *footpaths) {
      auto from = stations_[footpath->from()];
      auto to = stations_[footpath->to()];
      next_node_id_ = from->add_foot_edge(
          next_node_id_, make_foot_edge(from, to, footpath->duration()));
    }
  }

  void connect_reverse() {
    for (auto& station_node : sched_.station_nodes) {
      for (auto& station_edge : station_node->_edges) {
        station_edge._to->_incoming_edges.push_back(&station_edge);
        for (auto& edge : station_edge._to->_edges) {
          edge._to->_incoming_edges.push_back(&edge);
        }
      }
    }
  }

  void sort_connections() {
    for (auto& station_node : sched_.station_nodes) {
      for (auto& station_edge : station_node->_edges) {
        for (auto& edge : station_edge._to->_edges) {
          if (edge.empty()) {
            continue;
          }
          std::sort(begin(edge._m._route_edge._conns),
                    end(edge._m._route_edge._conns));
        }
      }
    }
  }

  int node_count() const { return next_node_id_; }

  unsigned duplicate_count_;

private:
  bitfield const& get_or_create_bitfield(String const* serialized_bitfield) {
    return map_get_or_create(bitfields_, serialized_bitfield, [&]() {
      return deserialize_bitset<BIT_COUNT>(
          {serialized_bitfield->c_str(), serialized_bitfield->Length()});
    });
  }

  void read_attributes(int day, Vector<Offset<Attribute>> const* attributes,
                       std::vector<attribute const*>& active_attributes) {
    active_attributes.clear();
    active_attributes.reserve(attributes->size());
    for (auto const& attr : *attributes) {
      if (!get_or_create_bitfield(attr->traffic_days()).test(day)) {
        continue;
      }
      auto const attribute_info = attr->info();
      active_attributes.push_back(
          get_or_create(attributes_, attribute_info, [&]() {
            auto new_attr = make_unique<attribute>();
            new_attr->_code = attribute_info->code()->str();
            new_attr->_str = attribute_info->text()->str();
            sched_.attributes.emplace_back(std::move(new_attr));
            return sched_.attributes.back().get();
          }));
    }
  }

  std::string const* get_or_create_direction(Direction const* dir) {
    if (dir == nullptr) {
      return nullptr;
    } else if (dir->station()) {
      return &sched_.stations[stations_[dir->station()]->_id]->name;
    } else /* direction text */ {
      return get_or_create(directions_, dir->text(), [&]() {
        sched_.directions.emplace_back(
            make_unique<std::string>(dir->text()->str()));
        return sched_.directions.back().get();
      });
    }
  }

  provider const* get_or_create_provider(Provider const* p) {
    if (p == nullptr) {
      return nullptr;
    } else {
      return get_or_create(providers_, p, [&]() {
        sched_.providers.emplace_back(make_unique<provider>(
            provider(p->short_name()->str(), p->long_name()->str(),
                     p->full_name()->str())));
        return sched_.providers.back().get();
      });
    }
  }

  int get_or_create_category_index(Category const* c) {
    return get_or_create(categories_, c, [&]() {
      int index = sched_.categories.size();
      sched_.categories.push_back(make_unique<category>(
          category(c->name()->str(), static_cast<uint8_t>(c->output_rule()))));
      return index;
    });
  }

  int get_or_create_platform(int day,
                             Vector<Offset<Platform>> const* platforms) {
    static constexpr int NO_TRACK = 0;
    if (sched_.tracks.empty()) {
      sched_.tracks.push_back("");
    }

    if (platforms == nullptr) {
      return NO_TRACK;
    }

    auto track_it = std::find_if(
        std::begin(*platforms), std::end(*platforms),
        [&](Platform const* track) {
          return get_or_create_bitfield(track->bitfield()).test(day);
        });
    if (track_it == std::end(*platforms)) {
      return NO_TRACK;
    } else {
      auto name = track_it->name()->str();
      return get_or_create(tracks_, name, [&]() {
        int index = sched_.tracks.size();
        sched_.tracks.push_back(name);
        return index;
      });
    }
  }

  std::unique_ptr<route> create_route(Route const* r) {
    auto route_index = ++next_route_index_;
    auto const& stops = r->stations();
    auto const& in_allowed = r->in_allowed();
    auto const& out_allowed = r->out_allowed();
    auto route_nodes = make_unique<route>();
    edge* last_route_edge = nullptr;
    for (unsigned stop_idx = 0; stop_idx < stops->size(); ++stop_idx) {
      auto section = add_route_section(
          route_index, stops->Get(stop_idx), in_allowed->Get(stop_idx),
          out_allowed->Get(stop_idx), last_route_edge,
          stop_idx != stops->size() - 1);
      route_nodes->push_back(section.first);
      last_route_edge = section.second;

      if (stop_idx == 0) {
        sched_.route_index_to_first_route_node.push_back(
            section.first.route_node);
      }
    }
    return route_nodes;
  }

  std::pair<route_info, edge*> add_route_section(
      int route_index, Station const* from_stop, bool in_allowed,
      bool out_allowed, edge* last_route_edge, bool build_outgoing_route_edge,
      node* route_node = nullptr) {
    auto const& station_node = stations_[from_stop];
    auto station_id = station_node->_id;

    if (route_node == nullptr) {
      route_node = new node(station_node, next_node_id_++);
      route_node->_route = route_index;

      // Connect the new route node with the corresponding station node:
      // route -> station: edge cost = change time, interchange count
      // station -> route: free
      if (!in_allowed) {
        station_node->_edges.push_back(
            make_invalid_edge(station_node, route_node));
      } else {
        station_node->_edges.push_back(
            make_foot_edge(station_node, route_node));
      }
      if (!out_allowed) {
        route_node->_edges.push_back(
            make_invalid_edge(route_node, station_node));
      } else {
        route_node->_edges.push_back(
            make_foot_edge(route_node, station_node,
                           sched_.stations[station_id]->transfer_time, true));
      }
    }

    // Connect route nodes with route edges.
    int route_edge_index = -1;
    if (build_outgoing_route_edge) {
      route_edge_index = route_node->_edges.size();
      route_node->_edges.push_back(make_route_edge(route_node, nullptr, {}));
    }
    if (last_route_edge != nullptr) {
      last_route_edge->_to = route_node;
    }

    return {route_info(route_node, route_edge_index),
            &route_node->_edges.back()};
  }

  unsigned next_route_index_;
  std::map<Category const*, int> categories_;
  std::map<std::string, int> tracks_;
  std::map<AttributeInfo const*, attribute*> attributes_;
  std::map<String const*, std::string const*> directions_;
  std::map<Provider const*, provider const*> providers_;
  hash_map<Station const*, station_node*> stations_;
  std::map<Timezone const*, timezone const*> timezones_;
  hash_map<String const*, bitfield> bitfields_;
  hash_set<connection_info*,
           deep_ptr_hash<connection_info::hash, connection_info>,
           deep_ptr_eq<connection_info>> con_infos_;
  hash_set<connection*, deep_ptr_hash<connection::hash, connection>,
           deep_ptr_eq<connection>> connections_;
  unsigned next_node_id_;
  schedule& sched_;
  int first_day_, last_day_;
  bool apply_rules_;

  connection_info con_info_;
  connection con_;
};

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool unique_check, bool apply_rules) {
  scoped_timer timer("building graph");

  schedule_ptr sched(new schedule());
  sched->classes = class_mapping();
  sched->schedule_begin_ = from;
  sched->schedule_end_ = to;

  graph_builder builder(*sched.get(), serialized->interval(), from, to,
                        apply_rules);
  builder.add_stations(serialized->stations());
  builder.add_services(serialized->services());
  builder.add_footpaths(serialized->footpaths());
  builder.connect_reverse();
  builder.sort_connections();

  sched->node_count = builder.node_count();
  sched->lower_bounds = constant_graph(sched->station_nodes);
  sched->waiting_time_rules_ = load_waiting_time_rules(sched->categories);
  sched->schedule_begin_ -= SCHEDULE_OFFSET_MINUTES * 60;

  if (unique_check) {
    scoped_timer timer("unique check");
    duplicate_checker dup_checker(*sched);
    dup_checker.remove_duplicates();
    LOG(info) << "removed " << dup_checker.get_duplicate_count()
              << " duplicate events";
  }

  return sched;
}

}  // loader
}  // motis
