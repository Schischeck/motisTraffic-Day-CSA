// #include "motis/routes/prepare/relation_matcher.h"

// #include <limits>

// #include "motis/core/common/get_or_create.h"
// #include "motis/core/common/logging.h"
// #include "motis/loader/util.h"
// #include "motis/routes/prepare/point_rtree.h"

// namespace motis {
// namespace routes {

// relation_matcher::relation_matcher(motis::loader::Schedule const* sched,
//                                    std::string const& osm_file)
//     : sched_(sched), osm_file_(osm_file) {
//   load_osm();
// };

// void relation_matcher::load_osm() {
//   logging::scoped_timer scoped_timer("Loading osm data");
//   foreach_osm_relation(osm_file_, [&](auto&& relation) {
//     if (relation.id() != 1741581) {
//       return;
//     }
//     std::string const type = relation.get_value_by_key("type", "");
//     std::string const route = relation.get_value_by_key("route", "");
//     if ((type == "route" || type == "public_transport") &&
//         (route == "subway" || route == "light_rail" || route == "railway" ||
//          route == "train" || route == "tram" || route == "bus")) {

//       std::vector<way*> ways;
//       for (auto const& member : relation.members()) {
//         if (member.type() != osmium::item_type::way) {
//           continue;
//         }

//         ways.push_back(get_or_create(ways_, member.ref(), [&]() {
//                          return std::make_unique<way>(member.ref());
//                        }).get());
//       }
//       relations_.emplace_back(relation.id(), std::move(ways));
//     }
//   });

//   foreach_osm_way(osm_file_, [&](auto&& way) {
//     auto w = ways_.find(way.id());
//     if (w == end(ways_) ||
//         std::string(way.get_value_by_key("highway", "")) == "platform" ||
//         std::string(way.get_value_by_key("public_transport", "")) ==
//             "platform" ||
//         std::string(way.get_value_by_key("role", "")) == "stop") {
//       return;
//     }

//     w->second->resolved_ = true;
//     w->second->nodes_ =
//         loader::transform_to_vec(std::begin(way.nodes()), std::end(way.nodes()),
//                                  [](auto&& n) { return node{n.ref()}; });

//     for (auto& n : w->second->nodes_) {
//       pending_nodes_[n.id_].push_back(&n);
//     }
//   });

//   foreach_osm_node(osm_file_, [&](auto&& node)  {
//     auto it = pending_nodes_.find(node.id());
//     if (it != end(pending_nodes_)) {
//       for (auto& n : it->second) {
//         n->resolved_ = true;
//         n->pos_ = {node.location().lat(), node.location().lon()};
//       }
//     }
//   });

//   prepare_relations(relations_);

//   // prepare_relations(relations_, ways_, nodes_);
//   // std::vector<osm_relation> output;
//   // for (auto const& rel : relations_) {
//   //   if (!rel.second.broken) {
//   //     output.push_back(rel.second);
//   //   }
//   // }

//   // export_geojson(output, ways_, nodes_);
//   // LOG(logging::log_level::info) << "Relations: " << relations_.size();
//   // LOG(logging::log_level::info) << "Ways: " << ways_.size();
//   // LOG(logging::log_level::info) << "Nodes: " << nodes_.size();
//   // LOG(logging::log_level::info) << "Routes in schedule: "
//   //                               << sched_->routes()->size();
// }

// // bool relation_matcher::nodes_in_order(segment_match sm) {
// //   auto rel = relations_.find(sm.rel_);
// //   if (rel == end(relations_)) {
// //     return false;
// //   }
// //   auto nodes = rel->second.nodes_;
// //   auto it = std::find(begin(nodes), end(nodes), sm.station_nodes_[0]);
// //   for (auto const& n : sm.station_nodes_) {
// //     auto next_it = std::find(begin(nodes), end(nodes), n);
// //     if (next_it < it) {
// //       return false;
// //     }
// //     it = next_it;
// //   }
// //   return true;
// // }

// // void relation_matcher::find_perfect_matches() {
// //   logging::scoped_timer timer("Finding Perfect Matches");
// //   int count = 0;
// //   int result = 0;
// //   std::vector<int> matched_routes;
// //   for (auto const& rel : relations_) {
// //     std::vector<osmium::Location> locs;
// //     for (auto const& r : rel.second.nodes_) {
// //       locs.push_back(nodes_.at(r).location_);
// //     }
// //     auto rtree = make_point_rtree(locs, [](auto&& s) {
// //       return point_rtree::point{s.lon(), s.lat()};
// //     });
// //     auto route_count = 0;
// //     for (auto const& r : *sched_->routes()) {
// //       std::cout << count << "/" << relations_.size() << "|" << route_count
// //                 << "/" << sched_->routes()->size() << "|   " << result <<
// //                 "\r";
// //       if (std::find(std::begin(matched_routes), std::end(matched_routes),
// //                     route_count) != std::end(matched_routes)) {
// //         route_count++;
// //         continue;
// //       }
// //       bool matched = true;
// //       for (auto const& s : *r->stations()) {
// //         if (rtree.in_radius(s->lat(), s->lng(), 100).empty()) {
// //           matched = false;
// //           break;
// //         }
// //       }
// //       if (matched) {
// //         matched_routes.push_back(route_count);
// //         result++;
// //       }
// //       route_count++;
// //     }
// //     count++;
// //   }
// //   LOG(logging::log_level::info) << "Matched routes: " << result;
// // }

// // void relation_matcher::find_segment_matches() {
// //   logging::scoped_timer timer("Finding Segment Matches");
// //   std::cout << std::endl;
// //   std::vector<segment_match> segments;
// //   auto count = 0;
// //   for (auto const& rel : relations_) {
// //     std::vector<osm_node*> n;
// //     std::for_each(rel.second.nodes_.begin(), rel.second.nodes_.end(),
// //                   [&](auto const& r) { n.push_back(&nodes_.at(r)); });
// //     auto rtree = make_point_rtree(n, [](auto&& s) {
// //       return point_rtree::point{s->location_.lon(), s->location_.lat()};
// //     });
// //     int route_count = 0;
// //     for (auto const& r : *sched_->routes()) {
// //       std::cout << count << "/" << relations_.size() << "|" << route_count++
// //                 << "/" << sched_->routes()->size() << "|   " <<
// //                 segments.size()
// //                 << "\r";
// //       std::vector<std::string> stations;
// //       std::vector<int64_t> station_nodes;
// //       for (auto const& s : *r->stations()) {
// //         auto result = rtree.in_radius(s->lat(), s->lng(), 100);
// //         if (!result.empty()) {
// //           station_nodes.push_back(n[result[0]]->id_);
// //           stations.push_back(s->id()->str());
// //           continue;
// //         }
// //         if (stations.size() >= 2) {
// //           segment_match match(rel.first, stations, station_nodes);
// //           if (nodes_in_order(match)) {
// //             segments.push_back(match);
// //           }
// //         }
// //         stations.clear();
// //         station_nodes.clear();
// //       }
// //     }
// //     count++;
// //   }
// // }

// }  // namespace routes
// }  // namespace motis
