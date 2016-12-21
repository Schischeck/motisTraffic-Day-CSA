#include "motis/path/prepare/routing/path_routing.h"

#include "motis/path/prepare/routing/relation_strategy.h"
#include "motis/path/prepare/routing/stub_strategy.h"

namespace motis {
namespace path {

struct path_routing::impl {

  std::unique_ptr<stub_strategy> stub_strategy_;
  std::unique_ptr<relation_strategy> relation_strategy_;
  std::map<source_spec::category, routing_strategy*> class_to_strategy_;
};

path_routing::path_routing() : impl_(std::make_unique<path_routing::impl>()) {}
path_routing::~path_routing() = default;

std::vector<routing_strategy const*> path_routing::strategies_for() const {
  return {impl_->relation_strategy_.get(), impl_->stub_strategy_.get()};
}

path_routing make_path_routing() {
  path_routing r;

  return path_routing{};
}

}  // namespace path
}  // namespace motis