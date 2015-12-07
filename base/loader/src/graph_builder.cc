#include "motis/loader/graph_builder.h"

#include <set>
#include <cassert>
#include <functional>
#include <algorithm>
#include <unordered_set>

#define RANGES_SUPPRESS_IOTA_WARNING
#include "range/v3/all.hpp"

#include "parser/cstr.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"
#include "motis/core/common/hash_helper.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/price.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/timezone.h"

#include "motis/loader/wzr_loader.h"
#include "motis/loader/util.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/classes.h"
#include "motis/loader/timezone_util.h"
#include "motis/loader/duplicate_checker.h"

using namespace motis::logging;
using namespace flatbuffers;
using namespace ranges;

namespace motis {
namespace loader {

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
      : next_node_id_(-1),
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

  void add_rule_services(Vector<Offset<RuleService>> const* rule_services) {
    if (rule_services == nullptr) {
      return;
    }

    if (!apply_rules_) {
      // Build rule services separately.
      std::set<Service const*> built;
      for (auto const& rs : *rule_services) {
        for (auto const& r : *rs->rules()) {
          for (auto const& s : {r->service1(), r->service2()}) {
            if (built.find(s) != end(built)) {
              add_service(s);
              built.insert(s);
            }
          }
        }
      }
      return;
    }

    unsigned route_id = routes_.size();
    for (auto const& rule_service : *rule_services) {
      auto traffic_days = get_or_create_bitfield(
          rule_service->rules()->Get(0)->service1()->traffic_days());
      if (!accumulate(view::ints(first_day_, last_day_ + 1), false,
                      [&traffic_days](bool acc, int day) {
                        return acc || traffic_days.test(day);
                      })) {
        continue;
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
          printf("\n\n\nERROR:\n");
          printf("offending %d %d\n\n",
                 rule->service1()->sections()->Get(0)->train_nr(),
                 rule->service2()->sections()->Get(0)->train_nr());
          for (auto const& r : *rule_service->rules()) {
            auto t1 = r->service1()->sections()->Get(0)->train_nr();
            auto t2 = r->service2()->sections()->Get(0)->train_nr();
            printf("%d\t%d\t\t\t%p\t%p\n", t1, t2, r->service1(),
                   r->service2());
          }
          printf("\n\n\n");
          goto next;
        }

        it = rules.erase(it);
        first = false;
      }

    next:
      ++route_id;
    }
  }

  route const* add_remaining_merge_split_sections(
      bitfield const& traffic_days, int route_index, Service const* new_service,
      Rule const* r, route const* existing_service_route_nodes) {
    auto const& stops = new_service->route()->stations();
    auto merge_station_id = sched_.eva_to_station.at(r->eva_1()->str())->index;
    auto split_station_id = sched_.eva_to_station.at(r->eva_2()->str())->index;

    // Determine merge and split route nodes of the existing service.
    auto merge_rn_it = std::find_if(begin(*existing_service_route_nodes),
                                    end(*existing_service_route_nodes),
                                    [&merge_station_id](route_info const& n) {
                                      return n.route_node->get_station()->_id ==
                                             merge_station_id;
                                    });
    verify(merge_rn_it != end(*existing_service_route_nodes),
           "mss: service[1] doesn't contain merge station mentioned in rule");
    auto other_service_merge_idx =
        std::distance(begin(*existing_service_route_nodes), merge_rn_it);

    auto split_rn_it = std::find_if(begin(*existing_service_route_nodes),
                                    end(*existing_service_route_nodes),
                                    [&split_station_id](route_info const& n) {
                                      return n.route_node->get_station()->_id ==
                                             split_station_id;
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
              out_allowed->Get(stop_idx), nullptr,
              stop_idx != stops->size() - 1, split_rn_it->route_node);
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
          add_service_section(
              (*route_nodes)[stop_idx].get_route_edge(),
              new_service->sections()->Get(stop_idx),
              new_service->platforms()
                  ? new_service->platforms()->Get(stop_idx + 1)->arr_platforms()
                  : nullptr,
              new_service->platforms()
                  ? new_service->platforms()->Get(stop_idx)->dep_platforms()
                  : nullptr,
              new_service->times()->Get(stop_idx * 2 + 1),
              new_service->times()->Get(stop_idx * 2 + 2), traffic_days);
          break;

        default:;
      }
    }

    routes_mem_.emplace_back(std::move(route_nodes));
    return routes_mem_.back().get();
  }

  route const* add_service(Service const* service, int route_index = -1) {
    auto traffic_days = get_or_create_bitfield(service->traffic_days());
    if (!accumulate(view::closed_ints(first_day_, last_day_), false,
                    [&traffic_days](bool acc, int day) {
                      return acc || traffic_days.test(day);
                    })) {
      return nullptr;
    }

    auto route_nodes = get_or_create(
        routes_, service->route(), [this, &service, &route_index]() {
          routes_mem_.emplace_back(
              create_route(service->route(),
                           route_index == -1 ? routes_.size() : route_index));
          return routes_mem_.back().get();
        });

    std::unordered_set<uint32_t> train_nrs;
    auto offset = service->times()->Get(service->times()->size() - 2) / 1440;
    for (unsigned section_idx = 0; section_idx < service->sections()->size();
         ++section_idx) {
      add_service_section(
          &(*route_nodes)[section_idx].route_node->_edges[1],
          service->sections()->Get(section_idx),
          service->platforms()
              ? service->platforms()->Get(section_idx + 1)->arr_platforms()
              : nullptr,
          service->platforms()
              ? service->platforms()->Get(section_idx)->dep_platforms()
              : nullptr,
          service->times()->Get(section_idx * 2 + 1),
          service->times()->Get(section_idx * 2 + 2), traffic_days, offset);
      train_nrs.insert(service->sections()->Get(section_idx)->train_nr());
    }

    if (route_index != -1) {
      for (uint32_t train_nr : train_nrs) {
        auto& routes = sched_.train_nr_to_routes[train_nr];
        if (std::find(begin(routes), end(routes), route_index) == end(routes)) {
          routes.push_back(route_index);
        }
      }
    }

    return route_nodes;
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

  void add_service_section(edge* e, Section const* section,
                           Vector<Offset<Platform>> const* arr_platforms,
                           Vector<Offset<Platform>> const* dep_platforms,
                           int const dep_time, int const arr_time,
                           bitfield const& traffic_days, int offset = 0) {
    assert(e->type() == edge::ROUTE_EDGE);

    // Departure station and arrival station.
    auto& from = *sched_.stations[e->_from->get_station()->_id];
    auto& to = *sched_.stations[e->_to->get_station()->_id];

    // Expand traffic days.
    for (int day = std::max(0, first_day_ - offset); day <= last_day_; ++day) {
      if (!traffic_days.test(day)) {
        continue;
      }

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
      read_attributes(dep_day_index, section->attributes(),
                      con_info_.attributes);

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
      auto const* dep_st =
          sched_.stations.at(e->_from->get_station()->_id).get();
      auto const* arr_st = sched_.stations.at(e->_to->get_station()->_id).get();

      auto const dep_motis_time =
          compute_event_time(day, dep_time, dep_st->timez);
      auto const arr_motis_time =
          compute_event_time(day, arr_time, arr_st->timez);

      e->_m._route_edge._conns.emplace_back(
          dep_motis_time, arr_motis_time,
          set_get_or_create(connections_, &con_, [&]() {
            sched_.full_connections.emplace_back(make_unique<connection>(con_));
            return sched_.full_connections.back().get();
          }));

      // Count events.
      ++from.dep_class_events[con_.clasz];
      ++to.arr_class_events[con_.clasz];
    }
  }

  time compute_event_time(int day, int local_time, timezone const* tz) const {
    return tz ? tz->to_motis_time(day - first_day_, local_time)
              : (day - first_day_ + 1) * MINUTES_A_DAY + local_time;
  }

  void validate_events(int day, station const* dep_st, station const* arr_st,
                       time dep_motis_time, time arr_motis_time,
                       int dep_local_time, int arr_local_time,
                       String const* origin) {
    auto const is_invalid_dep_event =
        dep_st->timez && dep_st->timez->is_invalid_time(dep_motis_time);
    auto const is_invalid_arr_event =
        arr_st->timez && arr_st->timez->is_invalid_time(arr_motis_time);
    auto const is_negative_edge = dep_motis_time > arr_motis_time;

    if (is_invalid_dep_event || is_invalid_arr_event || is_negative_edge) {
      LOG(emrg) << "[" << origin->c_str() << "]";
    }
    if (is_invalid_dep_event) {
      LOG(emrg) << "invalid departure time for (loader) day_idx: " << day;
    }
    if (is_invalid_arr_event) {
      LOG(emrg) << "invalid arrival time for (loader) day_idx: " << day;
    }
    if (is_negative_edge) {
      LOG(emrg) << "negative edge at section (" << dep_st->eva_nr << ","
                << arr_st->eva_nr << ")";
      LOG(emrg) << dep_local_time << "--local_time-->" << arr_local_time;
      LOG(emrg) << dep_motis_time << "--motis_time-->" << arr_motis_time;
    }
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

  std::unique_ptr<route> create_route(Route const* r, int route_index) {
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

  std::map<Category const*, int> categories_;
  std::map<Route const*, route const*> routes_;
  std::vector<std::unique_ptr<route>> routes_mem_;
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
  for (auto const& service : *serialized->services()) {
    builder.add_service(service);
  }
  builder.add_rule_services(serialized->rule_services());
  builder.add_footpaths(serialized->footpaths());
  builder.connect_reverse();
  builder.sort_connections();

  sched->node_count = builder.node_count();
  sched->lower_bounds = constant_graph(sched->station_nodes);
  sched->waiting_time_rules_ = load_waiting_time_rules(sched->categories);
  sched->schedule_begin_ -= SCHEDULE_OFFSET;

  if (unique_check) {
    scoped_timer timer("unique check");
    duplicate_checker dup_checker(*sched, false);
    dup_checker.remove_duplicates();
    LOG(info) << "removed " << dup_checker.get_duplicate_count()
              << " duplicate events";
  }

  return sched;
}

}  // loader
}  // motis
