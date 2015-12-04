#include "motis/loader/duplicate_checker.h"

#include <cinttypes>
#include <vector>
#include <functional>

#include "motis/loader/timezone_util.h"

namespace motis {
namespace loader {
//
// struct key {
//  light_connection const* lc;
//  node* route_node;
//  uint32_t connection_idx;
//};
//
// bool ascending_arrival(key const& k1, key const& k2) {
//  return k1.lc->a_time < k2.lc->a_time;
//}
//
// bool ascending_departure(key const& k1, key const& k2) {
//  return k1.lc->d_time < k2.lc->d_time;
//}
//
// void collect_arr_keys(station_node const* s, std::vector<key>& acc) {
//  for (auto const* r : s->get_incoming_route_nodes()) {
//    for (auto const* e : r->_incoming_edges) {
//      if (e->type() == edge::type::ROUTE_EDGE) {
//        unsigned connection_idx = 0;
//        for (auto const& lc : e->_m._route_edge._conns) {
//          // acc.push_back({&lc, r, connection_idx});
//          ++connection_idx;
//        }
//      }
//    }
//  }
//}
//
// void collect_dep_keys(station_node const* s, std::vector<key>& acc) {
//  for (auto const* r : s->get_route_nodes()) {
//    for (auto const& e : r->_edges) {
//      if (e.type() == edge::type::ROUTE_EDGE) {
//        unsigned connection_idx = 0;
//        for (auto const& lc : e._m._route_edge._conns) {
//          // acc.push_back({&lc, r, connection_idx});
//          ++connection_idx;
//        }
//      }
//    }
//  }
//}
//
// void for_each_key(std::vector<key>& keys,
//                  std::function<void(key const&, key const&)> action) {
//  for (unsigned i = 1; i < keys.size(); ++i) {
//    auto const& k1 = keys.at(i - 1);
//    auto const& k2 = keys.at(i);
//    action(std::cref(k1), std::ref(k2));
//  }
//}
//
// bool is_duplicate_event(time t1, time t2,  //
//                        light_connection const* lc1,
//                        light_connection const* lc2) {
//  return t1 == t2 &&
//         lc1->_full_con->con_info->train_nr ==
//             lc2->_full_con->con_info->train_nr &&
//         lc1->_full_con->con_info->line_identifier ==
//             lc2->_full_con->con_info->line_identifier;
//}
//
// void handle_duplicates(schedule const& sched, station_node const* s, time t1,
//                       time t2, key const& k1, key const& k2,
//                       std::set<duplicate>& duplicates) {
//  if (is_duplicate_event(t1, t2, k1.lc, k2.lc)) {
//    duplicates.emplace(k1.lc->_full_con->con_info, k1.route_node,
//                       k1.connection_idx);
//    duplicates.emplace(k2.lc->_full_con->con_info, k2.route_node,
//                       k2.connection_idx);
//  }
//}
//
// std::set<duplicate> duplicate_checker::check(station_node const* s) {
//  std::set<duplicate> duplicates;
//
//  std::vector<key> keys;
//  collect_arr_keys(s, keys);
//  std::sort(begin(keys), end(keys), ascending_arrival);
//  for_each_key(keys, [&](key const& k1, key const& k2) {
//    handle_duplicates(schedule_, s, k1.lc->a_time, k2.lc->a_time, k1, k2,
//                      duplicates);
//  });
//
//  keys.clear();
//  collect_dep_keys(s, keys);
//  std::sort(begin(keys), end(keys), ascending_departure);
//  for_each_key(keys, [&](key const& k1, key const& k2) {
//    handle_duplicates(schedule_, s, k1.lc->d_time, k2.lc->d_time, k1, k2,
//                      duplicates);
//  });
//
//  return duplicates;
//}
////
//// unsigned get_offset(std::map<node const*, unsigned>& offsets, node const*
///n)
//// {
////  //  auto it = offsets.find(n);
////  //  if (it == end(it)) {
////  //  }
////  //  auto const next_offset = get_or_create(offsets, n, []() { return 0;
////  })++;
////  //  offsets[origin]
////}
////
//// void duplicate_checker::remove_duplicates() {
////  //  std::map<node const*, unsigned> offsets;
////  //  for (auto const station_node_ptr : schedule_.station_nodes) {
////  //    for (auto const& duplicate : check(station_node_ptr.get())) {
////  //      //      cleanup_backward(duplicate.route_node, offsets);
////  //      //      cleanup_forward(duplicate.route_node, offsets);
////  //    }
////  //  }
////}

}  // namespace loader
}  // namespace motis
