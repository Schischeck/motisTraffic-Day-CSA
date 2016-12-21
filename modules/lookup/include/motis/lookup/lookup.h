#pragma once

#include <memory>

#include "geo/point_rtree.h"

#include "motis/module/module.h"

namespace motis {
namespace lookup {

struct lookup final : public motis::module::module {
  lookup();
  ~lookup() override;

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

  std::unique_ptr<geo::point_rtree> station_geo_index_;
};

}  // namespace lookup
}  // namespace motis
