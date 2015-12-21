#include "motis/loader/rule_service_graph_builder.h"

#include <map>
#include <set>
#include <vector>
#include <queue>
#include <algorithm>

#include "motis/core/schedule/price.h"

#include "motis/loader/util.h"

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

struct participant {
  Service const* service;
  int section_idx;
};

typedef std::pair<route_info, std::vector<participant>> service_section;
typedef std::pair<Service const*, Rule const*> neighbor;

struct rule_service_section_builder {
  rule_service_section_builder(graph_builder& gb, RuleService const* rs)
      : gb_(gb), neighbors_(build_neighbors(rs)) {}

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
          add_service(r->service2());
        }
      }
    }
  }

  void add_service(Service const* service) {
    auto& sections = sections_[service];
    sections.resize(service->sections()->size());
    add_service(service, 0, service->sections()->size(), 0,
                service->sections()->size(), sections, {});
  }

  void add_service(Service const* service,  //
                   int from_idx, int to_idx,  //
                   int src_from_idx, int src_to_idx,  //
                   std::vector<service_section*>& sections,
                   std::set<Service const*> visited) {
    visited.emplace(service);

    // Recursive add_service call for each neighbor.
    for (auto const& neighbor : neighbors_[service]) {
      Rule const* rule;
      Service const* neighbor_service;
      std::tie(neighbor_service, rule) = neighbor;

      if (visited.find(neighbor_service) != end(visited)) {
        continue;
      }

      auto rule_service_from = stop_index_of(service, rule->from());
      auto rule_service_to = stop_index_of(service, rule->to());
      auto rule_neighbor_from = stop_index_of(neighbor_service, rule->from());
      auto rule_neighbor_to = stop_index_of(neighbor_service, rule->to());

      auto service_from = std::max(rule_service_from, from_idx);
      auto service_to = std::min(rule_service_to, to_idx);
      auto neighbor_from =
          rule_neighbor_from + (service_from - rule_service_from);
      auto neighbor_to = rule_neighbor_to + (service_to - rule_service_to);

      if (service_from < service_to) {
        add_service(neighbor_service, neighbor_from, neighbor_to, src_from_idx,
                    src_to_idx, sections, visited);
      }
    }

    // Add service as participant to the specified sections.
    for (int src_section_idx = src_from_idx, neighbor_section_idx = from_idx;
         src_section_idx < src_to_idx;
         ++src_section_idx, ++neighbor_section_idx) {
      if (sections[src_section_idx] == nullptr) {
        section_mem_.emplace_back(new service_section());
        sections[src_section_idx] = section_mem_.back().get();
      }

      auto& section_participants = sections[src_section_idx]->second;
      auto not_already_added =
          std::find_if(begin(section_participants), end(section_participants),
                       [&service](participant const& p) {
                         return p.service == service;
                       }) == end(section_participants);

      if (not_already_added) {
        section_participants.push_back(
            participant{service, neighbor_section_idx});
      }
    }
  }

  graph_builder& gb_;
  std::map<Service const*, std::vector<neighbor>> neighbors_;
  std::map<Service const*, std::vector<service_section*>> sections_;
  std::vector<std::unique_ptr<service_section>> section_mem_;
};

struct rule_service_route_builder {
  rule_service_route_builder(
      graph_builder& gb,
      std::map<Service const*, std::vector<service_section*>>& sections,
      unsigned route_id)
      : gb_(gb), sections_(sections), route_id_(route_id) {}

  void build_routes() {
    for (auto& entry : sections_) {
      build_route(entry.first, entry.second);
    }
  }

  void build_route(Service const* s, std::vector<service_section*>& sections) {
    assert(s->sections()->size() == sections.size());
    assert(std::none_of(begin(sections), end(sections),
                        [](service_section* ss) { return ss == nullptr; }));
    assert(
        std::all_of(begin(sections), end(sections), [&s](service_section* ss) {
          auto is_curr = [&s](participant const& p) { p.service == s; };
          auto contains_curr = std::find_if(begin(ss->second), end(ss->second),
                                            is_curr) != end(sections);
          return contains_curr;
        }));

    auto const& r = s->route();
    auto const& stops = r->stations();
    auto const& in_allowed = r->in_allowed();
    auto const& out_allowed = r->out_allowed();
    edge* last_route_edge = nullptr;
    for (unsigned stop_idx = 0; stop_idx < stops->size(); ++stop_idx) {
      if (stop_idx == stops->size() - 1) {
        // Last stop: there is no following section from stop_idx to stop_idx+1
        // Only build the last route node (no outgoing route edge).
        auto last_route_edge = sections[stop_idx - 1]->first.get_route_edge();
        if (last_route_edge->_to == nullptr) {
          gb_.add_route_section(
              route_id_, stops->Get(stop_idx), in_allowed->Get(stop_idx),
              out_allowed->Get(stop_idx), last_route_edge, false);
        }
      } else if (sections[stop_idx]->first.is_valid()) {
        // Section already built.
        continue;
      }

      auto section = gb_.add_route_section(
          route_id_, stops->Get(stop_idx), in_allowed->Get(stop_idx),
          out_allowed->Get(stop_idx), last_route_edge,
          stop_idx != stops->size() - 1);
      last_route_edge = section.get_route_edge();
    }
  }

  graph_builder& gb_;
  std::map<Service const*, std::vector<service_section*>>& sections_;
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
    auto traffic_days = gb_.get_or_create_bitfield(
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

    rule_service_route_builder route_builder(gb_, section_builder.sections_,
                                             route_id);
    route_builder.build_routes();
  }
}

}  // loader
}  // motis
