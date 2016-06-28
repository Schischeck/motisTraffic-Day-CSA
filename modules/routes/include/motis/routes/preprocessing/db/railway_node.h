#pragma once

#include <vector>

namespace motis {
namespace routes {

class railway_link;

class railway_node {
public:
  railway_node(std::string id, double lat, double lng, std::string ds100)
      : id_(std::move(id)), lat_(lat), lng_(lng), ds100_(ds100) {}

  std::string id_;
  double lat_;
  double lng_;
  std::string ds100_;
  std::vector<railway_link*> links_;
};

}  // namespace routes
}  // namespace motis
