#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace routes {

struct routes : public motis::module::module {
  routes();
  ~routes() override;

  std::string name() const override { return "routes"; }
  void init(motis::module::registry&) override;

private:
  void load_auxiliary_file();

  motis::module::msg_ptr id_train_routes(motis::module::msg_ptr const&);

  motis::module::msg_ptr resolve_route_osrm(schedule const&, trip const*);
  motis::module::msg_ptr resolve_route_stub(schedule const&, trip const*);

  motis::module::msg_ptr trip_to_osrm_request(schedule const&, trip const*);

  std::string aux_file_;
  std::map<std::string, std::vector<std::pair<double, double>>>
      extra_bus_stop_positions_;
};

}  // namespace routes
}  // namespace motis
