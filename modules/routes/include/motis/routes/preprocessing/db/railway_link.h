#pragma once
#include <vector>

namespace motis {
namespace routes {

class railway_node;

class railway_link {
public:
  railway_link(std::string id, int weight)
      : weight_(weight), id_(std::move(id)) {}

  int weight_;
  std::string id_;
  std::vector<double> coords_;
  std::vector<railway_node*> nodes_;
};

}  // namespace routes
}  // namespace motis
