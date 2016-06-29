#pragma once

#include <map>
#include <string>
#include <vector>

namespace motis {
namespace routes {

struct coord {
  coord(double lat, double lng) : lat_(lat), lng_(lng) {}
  double lat_, lng_;
};

struct railway_node;
struct railway_link;

struct railway_graph {
  std::vector<std::unique_ptr<railway_node>> nodes_;
  std::vector<std::unique_ptr<std::vector<coord>>> polylines_;

  std::map<std::string, std::vector<size_t>> ds100_to_node_;
};

struct railway_node {
  railway_node(size_t idx, std::string id, coord pos, std::string ds100)
      : idx_(idx),
        id_(std::move(id)),
        pos_(pos),
        ds100_(std::move(ds100)),
        extra_{nullptr} {}

  size_t idx_;

  std::string id_;
  coord pos_;
  std::string ds100_;

  std::vector<railway_link> links_;
  railway_node const* extra_;
};

struct railway_link {
  railway_link(std::string id, std::vector<coord> const* polyline, size_t dist,
               railway_node const* from, railway_node const* to)
      : id_(std::move(id)),
        polyline_(polyline),
        dist_(dist),
        from_(from),
        to_(to) {}

  std::string id_;

  std::vector<coord> const* polyline_;
  size_t dist_;

  railway_node const* from_;
  railway_node const* to_;
};

}  // namespace routes
}  // namespace motis
