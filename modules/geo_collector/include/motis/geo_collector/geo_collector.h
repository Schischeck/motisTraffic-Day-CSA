#pragma once

#include "motis/module/module.h"

namespace motis {
namespace geo_collector {

struct geo_collector : public motis::module::module {
  geo_collector();
  ~geo_collector() = default;

  std::string name() const override { return "geo_collector"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr sign_up(motis::module::msg_ptr const&);
  motis::module::msg_ptr submit_measurements(motis::module::msg_ptr const&);
  motis::module::msg_ptr submit_journey(motis::module::msg_ptr const&);
  motis::module::msg_ptr upload(motis::module::msg_ptr const&);

  std::string conninfo_;
};

}  // namespace geo_collector
}  // namespace motis
