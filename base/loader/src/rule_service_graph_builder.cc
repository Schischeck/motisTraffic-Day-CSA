#include "motis/loader/rule_service_graph_builder.h"

#include <map>
#include <set>
#include <vector>
#include <queue>
#include <algorithm>

#include "motis/core/schedule/price.h"

#include "motis/loader/util.h"
#include "motis/loader/duplicate_checker.h"
#include "motis/core/common/get_or_create.h"

namespace motis {
namespace loader {

using namespace flatbuffers;

int calculate_offset(RuleService const* rule_service) {
  int offset = 0;
  for (auto const& rule : *rule_service->rules()) {
    for (auto const& s : {rule->service1(), rule->service2()}) {
      auto last_stop_arrival_time = s->times()->Get(s->times()->size() - 2);
      offset = std::max(offset, last_stop_arrival_time / 1440);
    }
  }
  return offset;
}

bool has_active_days(bitfield const& traffic_days, int first, int last) {
  for (int day = first; day <= last; ++day) {
    if (traffic_days.test(day)) {
      return true;
    }
  }
  return false;
}

typedef std::pair<route_section, std::vector<participant>> service_section;
typedef std::pair<Service const*, Rule const*> neighbor;

struct rule_service_section_builder {
  rule_service_section_builder(graph_builder& gb, RuleService const* rs)
      : gb_(gb),
        neighbors_(build_neighbors(rs)),
        sections_(build_empty_sections(rs)) {}

  static std::map<Service const*, std::vector<neighbor>> build_neighbors(
      RuleService const* rs) {
    std::map<Service const*, std::vector<neighbor>> neighbors;
    for (auto const& r : *rs->rules()) {
      if (r->type() != RuleType_THROUGH) {
        neighbors[r->service1()].emplace_back(r->service2(), r);
        neighbors[r->service2()].emplace_back(r->service1(), r);
      }
    }
    return neighbors;
  }

  static std::map<Service const*, std::vector<service_section*>>
  build_empty_sections(RuleService const* rs) {
    std::map<Service const*, std::vector<service_section*>> sections;
    for (auto const& r : *rs->rules()) {
      sections.emplace(r->service1(), std::vector<service_section*>(
                                          r->service1()->sections()->size()));
      sections.emplace(r->service2(), std::vector<service_section*>(
                                          r->service2()->sections()->size()));
    }
    return sections;
  }

  static int stop_index_of(Service const* s, Station const* station) {
    auto const& stations = *s->route()->stations();
    auto it = std::find(std::begin(stations), std::end(stations), station);
    verify(it != std::end(stations), "rule station not found");
    return std::distance(std::begin(stations), it);
  }

  void build_sections(RuleService const* rs) {
    std::set<Service const*> built;
    for (auto const& r : *rs->rules()) {
      for (auto const& s : {r->service1(), r->service2()}) {
        if (built.insert(s).second) {
          add_service(s);
        }
      }
    }
  }

  void add_service(Service const* service) {
    add_service(service, 0, service->sections()->size(), 0,
                service->sections()->size(), sections_[service],
                std::set<Service const*>());
  }

  void add_service(Service const* service,  //
                   int from_idx, int to_idx,  //
                   int src_from_idx, int src_to_idx,  //
                   std::vector<service_section*>& sections,
                   std::set<Service const*> visited) {
    visited.emplace(service);

    // Recursive add_service call for each neighbor.
    for (auto const& neighbor : neighbors_[service]) {
      Rule const* rule = neighbor.second;
      Service const* neighbor_service = neighbor.first;

      if (visited.find(neighbor_service) != end(visited)) {
        continue;
      }

      auto rule_service_from = stop_index_of(service, rule->from());
      auto rule_service_to = stop_index_of(service, rule->to());
      auto rule_neighbor_from = stop_index_of(neighbor_service, rule->from());
      auto rule_neighbor_to = stop_index_of(neighbor_service, rule->to());

      auto new_service_from_idx = std::max(rule_service_from, from_idx);
      auto new_service_to_idx = std::min(rule_service_to, to_idx);

      if (new_service_from_idx >= new_service_to_idx) {
        continue;
      }

      auto neighbor_from =
          rule_neighbor_from + (new_service_from_idx - rule_service_from);
      auto neighbor_to =
          rule_neighbor_to + (new_service_to_idx - rule_service_to);

      auto src_from = src_from_idx + (new_service_from_idx - from_idx);
      auto src_to = src_to_idx + (new_service_to_idx - to_idx);

      if (src_from < src_to) {
        add_service(neighbor_service, neighbor_from, neighbor_to, src_from,
                    src_to, sections, visited);
      }
    }

    // Add service as participant to the specified sections.
    for (int src_section_idx = src_from_idx, service_section_idx = from_idx;
         src_section_idx < src_to_idx;
         ++src_section_idx, ++service_section_idx) {
      if (sections[src_section_idx] == nullptr) {
        section_mem_.emplace_back(new service_section());
        sections[src_section_idx] = section_mem_.back().get();
      }
      sections_[service][service_section_idx] = sections[src_section_idx];

      auto& section_participants = sections[src_section_idx]->second;
      auto not_already_added =
          std::find_if(begin(section_participants), end(section_participants),
                       [&service](participant const& p) {
                         return p.service == service;
                       }) == end(section_participants);

      if (not_already_added) {
        section_participants.emplace_back(service, service_section_idx);
      }
    }
  }

  graph_builder& gb_;
  std::map<Service const*, std::vector<neighbor>> neighbors_;
  std::map<Service const*, std::vector<service_section*>> sections_;
  std::vector<std::unique_ptr<service_section>> section_mem_;
};

struct lcon_time_adjuster {
  static void adjust(edge* prev_edge, edge* e) {
    auto& prev_lcons = prev_edge->_m._route_edge._conns;
    auto& curr_lcons = e->_m._route_edge._conns;

    for (int lcon_idx = 0; lcon_idx < static_cast<int>(prev_lcons.size());
         ++lcon_idx) {
      auto& prev_lcon = prev_lcons[lcon_idx];
      auto& curr_lcon = curr_lcons[lcon_idx];

      auto& last_arr = prev_lcon.a_time;
      auto& curr_dep = curr_lcon.d_time;
      auto& curr_arr = curr_lcon.a_time;

      if (last_arr > curr_dep) {
        curr_dep += 60;
      }

      if (curr_dep > curr_arr) {
        curr_arr += 60;
      }

      assert(last_arr <= curr_dep && curr_dep <= curr_arr);
    }
  }

  void process_following_route_edges(edge* e) {
    for (auto& following : e->_to->_edges) {
      if (!following.empty()) {
        adjust(e, &following);
        queue_.emplace(&following);
      }
    }
  }

  void adjust_times(node* first_route_node) {
    for (auto& following : first_route_node->_edges) {
      queue_.emplace(&following);
    }

    while (!queue_.empty()) {
      auto el = queue_.front();
      queue_.pop();
      process_following_route_edges(el);
    }
  }

  std::queue<edge*> queue_;
};

struct rule_service_route_builder {
  rule_service_route_builder(
      graph_builder& gb,  //
      bitfield const& traffic_days, int first_day, int last_day,
      std::map<Service const*, std::vector<service_section*>>& sections,
      unsigned route_id)
      : gb_(gb),
        traffic_days_(traffic_days),
        first_day_(first_day),
        last_day_(last_day),
        sections_(sections),
        route_id_(route_id) {}

  void build_routes() {
    for (auto& entry : sections_) {
      build_route(entry.first, entry.second);
    }
  }

  void adjust_times() {
    for (auto& entry : sections_) {
      lcon_time_adjuster().adjust_times(entry.second[0]->first.from_route_node);
    }
  }

  std::array<trip*, 16> get_or_create_trips(
      std::array<participant, 16> const& services, int day_idx) {
    std::array<trip*, 16> trips;
    std::transform(begin(services), end(services), begin(trips),
                   [this, day_idx](participant const& p) -> trip* {
                     if (p.service == nullptr) {
                       return nullptr;
                     } else {
                       return get_or_create(
                           trips_, std::make_pair(p.service, day_idx), [&]() {
                             return gb_.register_service(p.service, day_idx);
                           });
                     }
                   });
    return trips;
  }

  std::vector<light_connection> build_connections(
      service_section const& section) {
    auto participants = section.second;
    std::sort(begin(participants), end(participants));

    std::array<participant, 16> services;
    std::copy(begin(participants), end(participants), begin(services));

    std::vector<light_connection> lcons;
    bool adjusted = false;
    for (int day_idx = first_day_; day_idx <= last_day_; ++day_idx) {
      if (traffic_days_.test(day_idx)) {
        lcons.push_back(
            gb_.section_to_connection(get_or_create_trips(services, day_idx),
                                      services, day_idx, 0, adjusted));
      }
    }
    return lcons;
  }

  void build_route(Service const* s, std::vector<service_section*>& sections) {
    assert(s->sections()->size() == sections.size());
    assert(std::none_of(begin(sections), end(sections),
                        [](service_section* ss) { return ss == nullptr; }));
    assert(std::all_of(
        begin(sections), end(sections), [&s, &sections](service_section* ss) {
          auto is_curr = [&s](participant const& p) { return p.service == s; };
          auto contains_curr = std::find_if(begin(ss->second), end(ss->second),
                                            is_curr) != end(ss->second);
          return contains_curr;
        }));

    auto const& r = s->route();
    auto const& stops = r->stations();
    auto const& in_allowed = r->in_allowed();
    auto const& out_allowed = r->out_allowed();
    for (unsigned i = 0; i < stops->size() - 1; ++i) {
      auto section_idx = i;
      if (sections[section_idx]->first.is_valid()) {
        continue;
      }

      auto from = section_idx;
      auto to = section_idx + 1;

      bool is_first = section_idx == 0;
      bool is_last = section_idx == stops->size() - 2;

      auto prev_section = is_first ? route_section() : sections[i - 1]->first;
      auto next_section = is_last ? route_section() : sections[i + 1]->first;

      sections[section_idx]->first = gb_.add_route_section(
          route_id_, build_connections(*sections[section_idx]),
          stops->Get(from), in_allowed->Get(from), out_allowed->Get(from),
          stops->Get(to), in_allowed->Get(to), out_allowed->Get(to),
          prev_section, next_section);
      assert(sections[section_idx]->first.is_valid());
    }
  }

  void connect_through_services(RuleService const* rs) {
    for (auto const& r : *rs->rules()) {
      if (r->type() == RuleType_THROUGH) {
        connect_route_nodes(r);
      }
    }
  }

  node* get_through_route_node(Service const* service, Station const* station,
                               bool source) {
    auto get_node = [source](service_section const* s) {
      return source ? s->first.to_route_node : s->first.from_route_node;
    };

    auto station_it = gb_.stations_.find(station);
    verify(station_it != end(gb_.stations_), "through station not found");
    auto station_node = station_it->second;

    auto& sections = sections_.at(service);
    auto it = std::find_if(begin(sections), end(sections),
                           [&](service_section const* s) {
                             return get_node(s)->get_station() == station_node;
                           });
    verify(it != end(sections), "through station not found");

    return get_node(*it);
  }

  void connect_route_nodes(Rule const* r) {
    auto s1_node = get_through_route_node(r->service1(), r->from(), true);
    auto s2_node = get_through_route_node(r->service2(), r->from(), false);
    s1_node->_edges.push_back(make_through_edge(s1_node, s2_node));
  }

  graph_builder& gb_;
  bitfield const& traffic_days_;
  int first_day_, last_day_;
  std::map<Service const*, std::vector<service_section*>>& sections_;
  std::map<std::pair<Service const*, int /* day index */>, trip*> trips_;
  unsigned route_id_;
};

rule_service_graph_builder::rule_service_graph_builder(graph_builder& gb)
    : gb_(gb) {}

void rule_service_graph_builder::add_rule_services(
    Vector<Offset<RuleService>> const* rule_services) {
  if (rule_services == nullptr) {
    return;
  }

  for (auto const& rule_service : *rule_services) {
    auto const& traffic_days = gb_.get_or_create_bitfield(
        rule_service->rules()->Get(0)->service1()->traffic_days());

    auto offset = calculate_offset(rule_service);
    auto first_day = std::max(0, gb_.first_day_ - offset);
    auto last_day = gb_.last_day_;
    if (!has_active_days(traffic_days, first_day, last_day)) {
      continue;
    }

    auto route_id = ++gb_.next_node_id_;

    rule_service_section_builder section_builder(gb_, rule_service);
    section_builder.build_sections(rule_service);

    rule_service_route_builder route_builder(
        gb_, traffic_days, first_day, last_day, section_builder.sections_,
        route_id);
    route_builder.build_routes();
    route_builder.connect_through_services(rule_service);
  }
}

}  // loader
}  // motis
