#include "motis/reliability/intermodal/individual_modes_container.h"

#include "motis/core/common/constants.h"
#include "motis/core/common/geo.h"

#include "motis/reliability/error.h"
#include "motis/reliability/intermodal/lookup.h"

namespace motis {
namespace reliability {
namespace intermodal {

template <typename Mode>
Mode find_id(std::vector<std::pair<int, Mode>> container, int const id) {
  auto it = std::find_if(container.begin(), container.end(),
                         [id](auto const& p) { return p.first == id; });
  if (it == container.end()) {
    throw std::system_error(error::failure);
  }
  return it->second;
}

motis::reliability::intermodal::bikesharing::bikesharing_info
get_bikesharing_info(individual_modes_container const& container,
                     int const mumo_id) {
  using namespace intermodal;
  if (!container.bikesharing_at_start_.empty() &&
      mumo_id >= container.bikesharing_at_start_.front().first &&
      mumo_id <= container.bikesharing_at_start_.back().first) {
    return find_id(container.bikesharing_at_start_, mumo_id);
  }
  if (!container.bikesharing_at_destination_.empty() &&
      mumo_id >= container.bikesharing_at_destination_.front().first &&
      mumo_id <= container.bikesharing_at_destination_.back().first) {
    return find_id(container.bikesharing_at_destination_, mumo_id);
  }
  throw std::system_error(error::failure);
}

mode_type individual_modes_container::get_mumo_type(int const id) const {
  if (id == -1) {
    return WALK;
  }
  if ((!bikesharing_at_start_.empty() &&
       id >= bikesharing_at_start_.front().first &&
       id <= bikesharing_at_start_.back().first) ||
      (!bikesharing_at_destination_.empty() &&
       id >= bikesharing_at_destination_.front().first &&
       id <= bikesharing_at_destination_.back().first)) {
    return BIKESHARING;
  }
  if (!taxis_.empty() && id >= taxis_.front().first &&
      id <= taxis_.back().first) {
    return TAXI;
  }
  if (!hotels_.empty() && id >= hotels_.front().first &&
      id <= hotels_.back().first) {
    return HOTEL;
  }
  if (!walks_at_start_.empty() && id >= walks_at_start_.front().first &&
      id <= walks_at_start_.back().first) {
    return WALK;
  }
  if (!walks_at_destination_.empty() &&
      id >= walks_at_destination_.front().first &&
      id <= walks_at_destination_.back().first) {
    return WALK;
  }
  throw std::system_error(error::failure);
}

void individual_modes_container::init_bikesharing(
    ReliableRoutingRequest const& req, bool const reliable_only,
    unsigned const max_duration, bool const pareto_filtering_for_bikesharing) {
  using namespace motis::reliability::intermodal::bikesharing;
  if (req.dep_is_intermodal()) {
    auto infos =
        retrieve_bikesharing_infos(true, req, reliable_only, max_duration,
                                   pareto_filtering_for_bikesharing);
    for (auto& info : infos) {
      bikesharing_at_start_.emplace_back(next_id(), std::move(info));
    }
  }
  if (req.arr_is_intermodal()) {
    auto infos =
        retrieve_bikesharing_infos(false, req, reliable_only, max_duration,
                                   pareto_filtering_for_bikesharing);
    for (auto& info : infos) {
      bikesharing_at_destination_.emplace_back(next_id(), std::move(info));
    }
  }
}

uint16_t foot_duration(double const& lat1, double const& lon1,
                       double const& lat2, double const& lon2) {
  constexpr unsigned sec_per_min = 60;
  return static_cast<uint16_t>(
      (geo_detail::distance_in_m(lat1, lon1, lat2, lon2) / WALK_SPEED) /
      sec_per_min);
}

void individual_modes_container::init_walks(ReliableRoutingRequest const& req) {
  auto init = [](std::vector<std::pair<int, walk>>& walks, double const& lat,
                 double const& lng) {
    auto lookup_msg = ask_lookup_module(lat, lng, MAX_WALK_DIST);
    using lookup::LookupGeoStationResponse;
    auto res = motis_content(LookupGeoStationResponse, lookup_msg);
    for (auto w : *res->stations()) {
      walks.emplace_back(
          0 /* dummy */,
          walk{w->id()->str(),
               foot_duration(lat, lng, w->pos()->lat(), w->pos()->lng())});
    }
  };
  if (req.dep_is_intermodal()) {
    init(walks_at_start_, req.dep_coord()->lat(), req.dep_coord()->lng());
  }
  if (req.arr_is_intermodal()) {
    init(walks_at_destination_, req.arr_coord()->lat(), req.arr_coord()->lng());
  }

  for (auto& w : walks_at_start_) {
    w.first = next_id();
  }
  for (auto& w : walks_at_destination_) {
    w.first = next_id();
  }
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
