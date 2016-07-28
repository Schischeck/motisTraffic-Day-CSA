#pragma once

#include <string>

#include "motis/module/module.h"

namespace motis {
namespace routes {

struct routes : public motis::module::module {
  routes();
  ~routes() override;

  std::string name() const override { return "routes"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr id_train_routes(motis::module::msg_ptr const&);

  motis::module::msg_ptr resolve_route_osrm(schedule const&, trip const*);
  motis::module::msg_ptr resolve_route_stub(schedule const&, trip const*);

  std::string routes_file_;
};

}  // namespace routes
}  // namespace motis
