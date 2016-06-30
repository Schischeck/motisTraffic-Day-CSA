#pragma once

#include <string>
#include <vector>

#include "parser/file.h"

#include "motis/module/module.h"
#include "motis/protocol/RoutesSections_generated.h"

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

  motis::module::msg_ptr routes_section(motis::module::msg_ptr const&);
  motis::module::msg_ptr all_sections(motis::module::msg_ptr const&);

  parser::buffer routes_buf_;
  std::string routes_file_;
  bool file_loaded_;
};

}  // namespace routes
}  // namespace motis
