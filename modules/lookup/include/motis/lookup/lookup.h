#pragma once

#include <memory>

#include "motis/module/module.h"
#include "motis/lookup/station_geo_index.h"

namespace motis {
namespace lookup {
class station_geo_index;

struct lookup final : public motis::module::module {
  lookup();
  ~lookup() override;

  std::string name() const override { return "lookup"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr lookup_station(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr lookup_stations(motis::module::msg_ptr const&) const;

  motis::module::msg_ptr lookup_station_events(
      motis::module::msg_ptr const&) const;

  motis::module::msg_ptr lookup_id_train(motis::module::msg_ptr const&) const;

  std::unique_ptr<station_geo_index> geo_index_;
};

}  // namespace lookup
}  // namespace motis
