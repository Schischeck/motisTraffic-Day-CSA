#pragma once

#include <memory>

#include "motis/module/module.h"

namespace motis {
namespace lookup {
class station_geo_index;

struct lookup final : public motis::module::module {
  lookup();
  ~lookup() override;

  std::string name() const override { return "lookup"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr lookup_station_id(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr lookup_station(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr lookup_stations(motis::module::msg_ptr const&) const;

  motis::module::msg_ptr lookup_station_events(
      motis::module::msg_ptr const&) const;

  motis::module::msg_ptr lookup_id_train(motis::module::msg_ptr const&) const;

  motis::module::msg_ptr lookup_meta_station(
      motis::module::msg_ptr const&) const;
  motis::module::msg_ptr lookup_meta_stations(
      motis::module::msg_ptr const&) const;

  motis::module::msg_ptr lookup_schedule_info() const;

  std::unique_ptr<station_geo_index> geo_index_;
};

}  // namespace lookup
}  // namespace motis
