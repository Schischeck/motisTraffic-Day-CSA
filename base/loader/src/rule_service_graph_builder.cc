#include "motis/loader/rule_service_graph_builder.h"

#include "motis/core/schedule/price.h"
#include "motis/core/common/logging.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {

using namespace flatbuffers;

rule_service_graph_builder::rule_service_graph_builder(graph_builder& gb)
    : gb_(gb) {}

void rule_service_graph_builder::add_rule_services(
    Vector<Offset<RuleService>> const* rule_services) {
  if (rule_services == nullptr) {
    return;
  }

  unsigned route_id = gb_.next_node_id_;
  for (auto const& rule_service : *rule_services) {
    auto traffic_days = gb_.get_or_create_bitfield(
        rule_service->rules()->Get(0)->service1()->traffic_days());

    for (int day = gb_.first_day_; day < gb_.last_day_; ++day) {
      if (traffic_days.test(day)) {
        continue;
      }
    }

    int iterations = 0;
    std::map<Service const*, route const*> service_route_nodes;
    bool first = true;
    std::vector<std::string> history;
    std::vector<Rule const*> rules(std::begin(*rule_service->rules()),
                                   std::end(*rule_service->rules()));
    auto it = begin(rules);
    while (!rules.empty()) {
      if (it == end(rules)) {
        it = begin(rules);
      }
      auto const& rule = *it;

      try {
        ++iterations;
        verify(iterations < 10000, "iterations maximum reached");

        if (rule->type() != RuleType_MERGE_SPLIT) {
          it = rules.erase(it);
          continue;
        }

        auto s1_it = service_route_nodes.find(rule->service1());
        auto s2_it = service_route_nodes.find(rule->service2());

        auto s1_new = s1_it == end(service_route_nodes);
        auto s2_new = s2_it == end(service_route_nodes);

        if (!first && s1_new && s2_new) {
          ++it;
          continue;
        }

        verify(s1_new || s2_new, "not both services already built");
        if (s1_new && !s2_new) {
          service_route_nodes.emplace(
              rule->service1(), add_remaining_merge_split_sections(
                                    traffic_days, route_id, rule->service1(),
                                    rule, s2_it->second));
        } else if (!s1_new && s2_new) {
          service_route_nodes.emplace(
              rule->service2(), add_remaining_merge_split_sections(
                                    traffic_days, route_id, rule->service2(),
                                    rule, s1_it->second));
        } else /* if (s1_new && s2_new) */ {
          std::tie(s1_it, std::ignore) = service_route_nodes.emplace(
              rule->service1(), add_service(rule->service1(), route_id));
          service_route_nodes.emplace(
              rule->service2(), add_remaining_merge_split_sections(
                                    traffic_days, route_id, rule->service2(),
                                    rule, s1_it->second));
        }
      } catch (...) {
        goto next;
      }

      it = rules.erase(it);
      first = false;
    }

  next:
    ++route_id;
  }
}

route const* rule_service_graph_builder::add_remaining_merge_split_sections(
    bitfield const& traffic_days, int route_index, Service const* new_service,
    Rule const* r, route const* existing_service_route_nodes) {
  auto const& stops = new_service->route()->stations();
  auto merge_station_id =
      gb_.sched_.eva_to_station.at(r->eva_1()->str())->index;
  auto split_station_id =
      gb_.sched_.eva_to_station.at(r->eva_2()->str())->index;

  // Determine merge and split route nodes of the existing service.
  auto merge_rn_it = std::find_if(
      begin(*existing_service_route_nodes), end(*existing_service_route_nodes),
      [&merge_station_id](route_info const& n) {
        return n.route_node->get_station()->_id == merge_station_id;
      });
  verify(merge_rn_it != end(*existing_service_route_nodes),
         "mss: service[1] doesn't contain merge station mentioned in rule");
  auto other_service_merge_idx =
      std::distance(begin(*existing_service_route_nodes), merge_rn_it);

  auto split_rn_it = std::find_if(
      begin(*existing_service_route_nodes), end(*existing_service_route_nodes),
      [&split_station_id](route_info const& n) {
        return n.route_node->get_station()->_id == split_station_id;
      });
  verify(split_rn_it != end(*existing_service_route_nodes),
         "mss: service[1] doesn't contain split station mentioned in rule");
#ifndef NDEBUG
  auto other_service_split_idx =
      std::distance(begin(*existing_service_route_nodes), split_rn_it);
#endif

  // Determine merge and split stop indices of the new service.
  auto merge_stop_it = std::find_if(
      std::begin(*stops), std::end(*stops),
      [&r](Station const* s) { return s->id()->str() == r->eva_1()->str(); });
  verify(merge_stop_it != std::end(*stops),
         "mss: service[2] doesn't contain merge station mentioned in rule");
  auto merge_stop_idx = std::distance(std::begin(*stops), merge_stop_it);

  auto split_stop_it = std::find_if(
      std::begin(*stops), std::end(*stops),
      [&r](Station const* s) { return s->id()->str() == r->eva_2()->str(); });
  verify(split_stop_it != std::end(*stops),
         "mss: service[2] doesn't contain split station mentioned in rule");
  auto split_stop_idx = std::distance(std::begin(*stops), split_stop_it);

  enum state {
    ENTRY,  // allows to skip the BUILD_END state (1st station = merge)
    BUILD,  // non-common part of both services -> build new service
    BUILD_END,  // merge station -> set edge target of last built edge
    SKIP,  // common part -> do nothing
    SKIP_END  // split station -> start building
  };
  auto next_state = [](state s, bool merge, bool split) {
    switch (s) {
      case ENTRY:
        if (merge) {
          return SKIP;
        } else {
          return BUILD;
        }

      case BUILD:
        if (merge) {
          return BUILD_END;
        } else {
          return BUILD;
        }
        break;

      case BUILD_END: return SKIP; break;

      case SKIP:
        if (split) {
          return SKIP_END;
        } else {
          return SKIP;
        }
        break;

      case SKIP_END: return BUILD; break;
    }
    assert(false);
    return ENTRY;
  };

  state s = ENTRY;
  edge* last_route_edge = nullptr;
  auto route_nodes = make_unique<route>();
  auto const& in_allowed = new_service->route()->in_allowed();
  auto const& out_allowed = new_service->route()->out_allowed();
  for (unsigned stop_idx = 0,
                other_service_route_node_idx = other_service_merge_idx;
       stop_idx < stops->size(); ++stop_idx, ++other_service_route_node_idx) {
    s = next_state(s, stop_idx == merge_stop_idx, stop_idx == split_stop_idx);
    route_info route_node(nullptr, -1);
    switch (s) {
      case ENTRY: assert(false && "state not accessible"); break;
      case BUILD:
        std::tie(route_node, last_route_edge) = add_route_section(
            route_index, stops->Get(stop_idx), in_allowed->Get(stop_idx),
            out_allowed->Get(stop_idx), last_route_edge,
            stop_idx != stops->size() - 1);
        break;

      case BUILD_END:
        route_node = *merge_rn_it;
        last_route_edge->_to = merge_rn_it->route_node;
        other_service_route_node_idx = other_service_merge_idx;
        break;

      case SKIP:
        route_node =
            (*existing_service_route_nodes)[other_service_route_node_idx];
        // add_merge_info(route_node.);
        break;

      case SKIP_END:
        assert(other_service_route_node_idx == other_service_split_idx);
        std::tie(route_node, last_route_edge) = add_route_section(
            route_index, stops->Get(stop_idx), in_allowed->Get(stop_idx),
            out_allowed->Get(stop_idx), nullptr, stop_idx != stops->size() - 1,
            split_rn_it->route_node);
        break;
    }

    verify(route_node.route_node != nullptr &&
               (stop_idx == stops->size() - 1 ||
                route_node.outgoing_route_edge_index != -1),
           "route node must be set and only last node has no outgoing edge: "
           "stop_idx=%d, #stops=%d, "
           "route_node.outgoing_route_edge_index=%d\n",
           stop_idx, stops->size(), route_node.outgoing_route_edge_index);
    route_nodes->push_back(route_node);
  }

  for (unsigned stop_idx = 0; stop_idx < stops->size() - 1; ++stop_idx) {
    s = next_state(s, stop_idx == merge_stop_idx, stop_idx == split_stop_idx);
    switch (s) {
      case BUILD:
      case SKIP_END:
        // TODO gb_.add_route_section()
        //        add_service_section(
        //            (*route_nodes)[stop_idx].get_route_edge(),
        //            new_service->sections()->Get(stop_idx),
        //            new_service->platforms()
        //                ? new_service->platforms()->Get(stop_idx +
        //                1)->arr_platforms()
        //                : nullptr,
        //            new_service->platforms()
        //                ?
        //                new_service->platforms()->Get(stop_idx)->dep_platforms()
        //                : nullptr,
        //            new_service->times()->Get(stop_idx * 2 + 1),
        //            new_service->times()->Get(stop_idx * 2 + 2),
        //            traffic_days);
        break;

      default:;
    }
  }

  routes_mem_.emplace_back(std::move(route_nodes));
  return routes_mem_.back().get();
}

route const* rule_service_graph_builder::add_service(Service const* service,
                                                     int route_index) {
  auto traffic_days = gb_.get_or_create_bitfield(service->traffic_days());
  for (int day = gb_.first_day_; day < gb_.last_day_; ++day) {
    if (traffic_days.test(day)) {
      return nullptr;
    }
  }

  //  auto& route_nodes = *get_or_create(
  //      gb_.routes_, service->route(), [this, &service, &route_index]() {
  //        routes_mem_.emplace_back(
  //            create_route(service->route(),
  //                         route_index == -1 ? routes_.size() : route_index));
  //        return routes_mem_.back().get();
  //      });

  auto offset = service->times()->Get(service->times()->size() - 2) / 1440;
  for (unsigned section_idx = 0; section_idx < service->sections()->size();
       ++section_idx) {
    // TODO gb_.add_route_section()
    //    add_service_section(
    //        &(*route_nodes)[section_idx].route_node->_edges[1],
    //        service->sections()->Get(section_idx),
    //        service->platforms()
    //            ? service->platforms()->Get(section_idx + 1)->arr_platforms()
    //            : nullptr,
    //        service->platforms()
    //            ? service->platforms()->Get(section_idx)->dep_platforms()
    //            : nullptr,
    //        service->times()->Get(section_idx * 2 + 1),
    //        service->times()->Get(section_idx * 2 + 2), traffic_days, offset);
  }

  //  return &route_nodes;
  return nullptr;
}

}  // loader
}  // motis
