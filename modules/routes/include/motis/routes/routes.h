#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "boost/optional.hpp"

#include "motis/module/module.h"

#include "motis/geo/latlng.h"

namespace motis {
namespace routes {

struct auxiliary_data;

struct routes : public motis::module::module {
  routes();
  ~routes() override;

  std::string name() const override { return "routes"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr id_train_routes(motis::module::msg_ptr const&);

  boost::optional<motis::module::msg_ptr> resolve_prepared_route(
      schedule const&, trip const*);

  motis::module::msg_ptr resolve_route_osrm(schedule const&, trip const*);
  motis::module::msg_ptr resolve_route_stub(schedule const&, trip const*);

  motis::module::msg_ptr trip_to_osrm_request(schedule const&, trip const*);

  std::string aux_file_;
  std::unique_ptr<auxiliary_data> aux_;
};

}  // namespace routes
}  // namespace motis
