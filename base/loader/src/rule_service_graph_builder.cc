#include "motis/loader/rule_service_graph_builder.h"

#include <map>
#include <vector>
#include <queue>
#include <algorithm>

#include "motis/core/schedule/price.h"
#include "motis/core/common/logging.h"

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

    add_rule_service(rule_service);
  }
}

struct rule_service_builder {
  struct participant {
    Service const* service;
    std::size_t section_idx;
  };

  struct service_range {
    Service const* service;
    std::size_t src_from_idx, src_to_idx;
    std::size_t neighbor_from_idx, neighbor_to_idx;
    Station const *from, *to;
  };

  typedef std::vector<participant> service_section;
  typedef std::pair<Service const*, Rule const*> neighbor;

  rule_service_builder(graph_builder& gb, RuleService const* rs)
      : gb_(gb),
        route_id_(++gb.next_node_id_),
        neighbors_(build_neighbors(rs)) {
    build_sections(rs);
  }

  static std::map<Service const*, std::vector<neighbor>> build_neighbors(
      RuleService const* rs) {
    std::map<Service const*, std::vector<neighbor>> neighbors;
    for (auto const& r : *rs->rules()) {
      neighbors[r->service1()].emplace_back(r->service2(), r);
      neighbors[r->service2()].emplace_back(r->service1(), r);
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
    for (auto const& r : *rs->rules()) {
      build_sections(r->service2());
      build_sections(r->service1());
    }
  }

  void build_sections(Service const* s) {
    auto const& stations = s->route()->stations();
    std::queue<service_range> queue;
    queue.push(service_range{s,  // extracted "neighbor"
                             0, stations->size() - 1,  // source from, to
                             0, stations->size() - 1,  // neighbor from, to
                             stations->Get(0),
                             stations->Get(stations->size() - 1)});

    auto& sections =
        sections_.emplace(s,
                          std::vector<service_section*>(s->sections()->size()))
            .first->second;

    while (!queue.empty()) {
      auto curr = queue.front();
      queue.pop();

      for (unsigned src_section_idx = curr.src_from_idx,
                    neighbor_section_idx = curr.neighbor_from_idx;
           src_section_idx < curr.src_to_idx;
           ++src_section_idx, ++neighbor_section_idx) {
        assert(neighbor_section_idx < curr.neighbor_to_idx);

        if (sections[src_section_idx] == nullptr) {
          section_mem_.emplace_back(new service_section());
          sections[src_section_idx] = section_mem_.back().get();
        }

        sections[src_section_idx]->push_back(
            participant{curr.service, neighbor_section_idx});
      }

      for (auto const& neighbor : neighbors_[curr.service]) {
        if (!considered(curr.service, neighbor.first)) {
        }
      }
    }
  }

  bool considered(Service const* s1, Service const* s2) const {
    auto it = considered_.find(s1);
    if (it == end(considered_)) {
      return false;
    }
    return std::find(begin(it->second), end(it->second), s2) != end(it->second);
  }

  graph_builder& gb_;
  unsigned route_id_;
  std::map<Service const*, std::vector<Service const*>> considered_;
  std::map<Service const*, std::vector<neighbor>> neighbors_;
  std::map<Service const*, std::vector<service_section*>> sections_;
  std::vector<std::unique_ptr<service_section>> section_mem_;
};

}  // loader
}  // motis
