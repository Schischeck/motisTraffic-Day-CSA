#pragma once

#include <string>
#include <vector>

#include "motis/module/module.h"
#include "parser/file.h"

#include "motis/protocol/RoutesSections_generated.h"

namespace motis {
namespace routes {
struct routes : public motis::module::module {
  routes();
  ~routes() override;

  std::string name() const override { return "routes"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr routes_section(motis::module::msg_ptr const& m);

  parser::buffer routes_buf_;
  std::string routes_file_;
  bool file_loaded_;
};
}  // namespace routes
}  // namespace motis
