#pragma once

#include <map>
#include <string>
#include <vector>

namespace motis {
namespace routes {

struct railway_node;
struct railway_link;

struct railway_graph {
public:
  std::vector<std::unique_ptr<railway_node>> nodes_;
  std::map<std::string, std::unique_ptr<railway_link>> links_;
  std::map<std::string, railway_node*> ds100_to_node_;
};

struct railway_node {
public:
  railway_node(std::string id, double lat, double lng, std::string ds100)
      : id_(std::move(id)), lat_(lat), lng_(lng), ds100_(std::move(ds100)) {}

  std::string id_;
  double lat_, lng_;
  std::string ds100_;
  std::vector<railway_link*> links_;
};

struct railway_link {
public:
  railway_link(std::string id, int weight)
      : weight_(weight), id_(std::move(id)) {}

  int weight_;
  std::string id_;
  std::vector<double> coords_;
  std::vector<railway_node*> nodes_;
};


} // namespace routes
} // namespace motis
