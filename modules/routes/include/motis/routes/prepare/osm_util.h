#pragma once

#include <algorithm>

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_iterator.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

#include "motis/core/common/geo.h"

using namespace rapidjson;

namespace motis {
namespace routes {

template <typename F>
void foreach_osm_node(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::node);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Node&>(*it));
  }
}

template <typename F>
void foreach_osm_way(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::way);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Way&>(*it));
  }
}

template <typename F>
void foreach_osm_relation(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::relation);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Relation&>(*it));
  }
}

struct node {
  node() = default;
  node(int64_t id) : id_(id), resolved_(false) {}

  int64_t id_;
  bool resolved_;
  geo_detail::latlng pos_;
};

struct way {
  way(int64_t id) : id_(id), resolved_(false) {}

  node const& head() const { return nodes_.back(); }
  node const& tail() const { return nodes_.front(); }

  int64_t id_;
  bool resolved_;
  std::vector<node> nodes_;
};

struct relation {
  relation(int64_t id, std::vector<way*> ways)
      : id_(id), ways_(std::move(ways)) {}

  int64_t id_;
  std::vector<way*> ways_;
};

// inline std::vector<way> expand_segment(
//     motis::routes::way const& way,
//     std::vector<motis::routes::way> const& ways) {
//   std::vector<motis::routes::way> seg = {way};
//   std::vector<int64_t> used = {way.id_};
//   while (used.size() < ways.size()) {
//     std::vector<motis::routes::way> add_front;
//     std::vector<motis::routes::way> add_back;
//     auto& front = seg.front().front_;
//     auto& back = seg.back().back_;
//     for (auto const& w : ways) {
//       if (std::find(begin(used), end(used), w.id_) == end(used)) {
//         if (w.back_ == front) {
//           add_front.push_back(w);
//           used.push_back(w.id_);
//         }
//         if (w.front_ == back) {
//           add_back.push_back(w);
//           used.push_back(w.id_);
//         }
//       }
//     }
//     if (add_back.size() > 1 || add_front.size() > 1) {
//       return {};
//     }
//     if (add_back.size() == 1) {
//       seg.push_back(add_back.front());
//     }
//     if (add_front.size() == 1) {
//       seg.insert(begin(seg), add_front.front());
//     }
//     if (add_front.empty() && add_back.empty()) {
//       break;
//     }
//   }
//   return seg;
// }

constexpr auto kIdentity = -std::numeric_limits<double>::infinity();
constexpr auto kUnreachable = std::numeric_limits<double>::infinity();

inline std::vector<std::vector<double>> distance_matrix(
    std::vector<way*> const& ways) {
  auto distance = [](node const& from, node const& to) {
    if (from.id_ == to.id_) {
      return kIdentity;
    }

    double const dist = geo_detail::distance_in_m(from.pos_, to.pos_);
    if (dist < 10) {
      return dist;
    }

    return kUnreachable;
  };

  std::vector<std::vector<double>> distances;
  for (auto i = 0u; i < ways.size(); ++i) {
    std::vector<double> head_distances;
    std::vector<double> tail_distances;

    auto const from_head = ways[i]->head();
    auto const from_tail = ways[i]->tail();

    for (auto j = 0u; j < ways.size(); ++j) {
      if (i == j) {
        head_distances.push_back(kUnreachable);
        head_distances.push_back(kUnreachable);

        tail_distances.push_back(kUnreachable);
        tail_distances.push_back(kUnreachable);
        continue;
      }

      auto const to_head = ways[j]->head();
      auto const to_tail = ways[j]->tail();

      head_distances.push_back(distance(from_head, to_head));
      head_distances.push_back(distance(from_head, to_tail));

      tail_distances.push_back(distance(from_tail, to_head));
      tail_distances.push_back(distance(from_tail, to_tail));
    }

    distances.emplace_back(std::move(head_distances));
    distances.emplace_back(std::move(tail_distances));
  }

  return distances;
}

inline std::vector<latlng> extract_polyline(std::vector<std:vector<double>> const& distances, size_t edge, ) {



}


// TODO backwards search
inline std::vector<int64_t> aggregate_ways(std::vector<way*> const& ways) {
  auto distances = distance_matrix(ways);

  std::cout << std::endl;
  for (auto const& way : ways) {
    std::cout << way->id_ << std::endl;
  }

  std::cout << std::endl;
  for (auto i = 0u; i < distances.size(); ++i) {
    for (auto j = 0u; j < distances[i].size(); ++j) {
      std::cout << std::setw(8) << distances[i][j] << " ";
    }
    std::cout << std::endl;
  }

  for (auto i = 0u; i < ways.size(); ++i) {
    auto const& tail_distances = distances[2 * i + 1];
    if (std::any_of(begin(tail_distances), end(tail_distances),
                    [](double const& dist) { return dist != kUnreachable; })) {
      std::cout << "reachable" << ways[i]->id_ << std::endl;
      continue;
    }

    std::cout << "unreachable" << ways[i]->id_ << std::endl;

    // auto head_min = std::min_element(begin(tail_distances),
    // end(tail_distances),
    //                                  [](double const& dist) { return })
  }

  // while (true) {

  //   auto curr_segment = segments.front();
  //   segments.erase(begin(segments));

  //   boost::optional<std::pair<segment, order>> succ_candidate;
  //   for (auto const& segment : segments) {
  //     auto head_head_dist =
  //         geo_detail::distance_in_m(curr_segment.head(), segment.head());
  //     auto head_tail_dist =
  //         geo_detail::distance_in_m(curr_segment.head(), segment.tail());

  //     if (succ_candidate && (head_head_dist < 10 || head_tail_dist < 10)) {
  //       succ_candidate = {};
  //       break;
  //     }

  //     if (head_head_dist < 10 && head_head_dist < head_tail_dist) {
  //       succ_candidate = {segment, order::HEAD_HEAD};
  //     } else if (head_tail_dist < 10) {
  //       succ_candidate = {segment, order::HEAD_TAIL};
  //     }
  //   }

  //   if (succ_candidate) {
  //     auto merged = merge_segments(curr_segment, *succ_candidate);
  //   }
  // }

  // return result;

  return {};
}

// inline std::vector<int64_t> sort_ways(std::vector<way>& ways) {
//   std::vector<std::vector<way>> segments;
//   if (ways.empty()) {
//     return {};
//   }
//   while (!ways.empty()) {
//     auto& start = ways[0];
//     auto const& seg = expand_segment(start, ways);
//     if (seg.empty()) {
//       return {};
//     }
//     for (auto const& s : seg) {
//       auto w = std::find_if(begin(ways), end(ways),
//                             [s](auto&& w) { return w.id_ == s.id_; });
//       if (w != end(ways)) {
//         ways.erase(w);
//       }
//     }
//     segments.push_back(seg);
//   }
//   return sort_segments(segments);
// }

inline void prepare_relations(std::vector<relation> relations) {
  for (auto& relation : relations) {
    auto& ways = relation.ways_;

    for (auto& way : ways) {
      for (auto& node : way->nodes_) {
        if (!node.resolved_) {
          std::cout << "missing node" << node.id_ << std::endl;
        }
      }
    }

    ways.erase(std::remove_if(begin(ways), end(ways),
                              [](auto const& w) {
                                return !w->resolved_ || w->nodes_.size() < 2;
                              }),
               end(ways));

    aggregate_ways(ways);
  }
}

// inline void prepare_relations(std::map<int64_t, osm_relation>& relations,
//                               std::map<int64_t, osm_way> const& ways,
//                               std::map<int64_t, node>& nodes) {
// std::cout << std::endl;
// std::vector<int64_t> broken_relations;
// int count = 0;
// for (auto& rel : relations) {
//   std::cout << count << "/" << relations.size() << "\r";

//   std::vector<way> ways;

//   count++;
//   for (auto const& w : rel.second.ways_) {
//     auto const way = ways.find(w);
//     if (way != ways.end() && !way->second.nodes_.empty()) {
//       small_ways.emplace_back(way->second.id_, way->second.nodes_.front(),
//                               way->second.nodes_.back());
//       auto f = nodes.find(small_ways.back().front_);
//       auto b = nodes.find(small_ways.back().back_);
//       if (f != end(nodes) && b != end(nodes)) {
//         small_ways.back().f = f->second.location_;
//         small_ways.back().b = b->second.location_;
//       }
//     }
//   }
//   auto const& sorted_ways = sort_ways(small_ways);
//   if (sorted_ways.empty()) {
//     broken_relations.push_back(rel.first);
//     rel.second.broken = true;
//     continue;
//   }
//   rel.second.ways_.clear();
//   for (auto const& w : sorted_ways) {
//     auto way = ways.find(w);
//     if (way != end(ways)) {
//       rel.second.ways_.push_back(w);
//       rel.second.nodes_.insert(rel.second.nodes_.end(),
//                                way->second.nodes_.begin(),
//                                way->second.nodes_.end());
//     }
//   }
// }
// std::cout << "Broken relations: " << broken_relations.size() << std::endl;
// }

// inline void export_geojson(std::vector<osm_relation> const& rels,
//                            std::map<int64_t, osm_way> const& ways,
//                            std::map<int64_t, osm_node> const& nodes) {
//   std::cout << "EXPORT: " << rels.size() << std::endl;
//   FILE* fp = std::fopen("geo.json", "w");
//   char writeBuffer[65536];

//   rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
//   rapidjson::PrettyWriter<rapidjson::FileWriteStream> w(os);

//   w.StartObject();
//   w.String("type").String("FeatureCollection");
//   w.String("features").StartArray();

//   for (auto const& rel : rels) {
//     // for (auto const& goal : goal_idx) {
//     w.StartObject();
//     w.String("type").String("Feature");
//     w.String("properties").StartObject().EndObject();
//     w.String("geometry").StartObject();
//     w.String("type").String("LineString");
//     w.String("coordinates").StartArray();

//     for (auto const& way : rel.ways_) {
//       auto wy = ways.find(way);
//       if (wy != end(ways)) {
//         for (auto const& n : wy->second.nodes_) {
//           auto node = nodes.find(n);
//           if (node != end(nodes)) {
//             w.StartArray();
//             w.Double(node->second.location_.lon(), 9);
//             w.Double(node->second.location_.lat(), 9);
//             w.EndArray();
//           }
//         }
//       }
//     }
//     w.EndArray();
//     w.EndObject();
//     w.EndObject();
//   }

//   w.EndArray();
//   w.EndObject();
// }
// struct segment_match {
//   segment_match(int64_t rel, std::vector<std::string> stations,
//                 std::vector<int64_t> station_nodes)
//       : rel_(rel), stations_(stations), station_nodes_(station_nodes){};

//   int64_t rel_;
//   std::vector<std::string> stations_;
//   std::vector<int64_t> station_nodes_;
// };
}
}
