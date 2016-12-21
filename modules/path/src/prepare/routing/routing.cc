#include "motis/path/prepare/routing/routing.h"

#include "motis/path/prepare/routing/stub_strategy.h"
#include "motis/path/prepare/routing/relation_strategy.h"

namespace motis {
namespace path {

struct routing::impl {

  std::unique_ptr<stub_strategy> stub_strategy_;
  std::unique_ptr<relation_strategy> relation_strategy_;
  std::map<source_spec::category, routing_strategy*> class_to_strategy_;
};

routing::routing() : impl_(std::make_unique<routing::impl>()) {}
routing::~routing() = default;

std::vector<routing_strategy const*> strategies_for() const {
  return {impl_->relation_strategy_.get(), impl_->stub_strategy_.get()};
}


routing make_routing() {
  routing r;

  return r;
}

}  // namespace path
}  // namespace motis