#include "motis/reliability/intermodal/individual_modes_container.h"

#include "motis/reliability/error.h"

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
  throw std::system_error(error::failure);
}

void individual_modes_container::init_bikesharing(
    ReliableRoutingRequest const& req, unsigned const max_duration,
    bool const pareto_filtering_for_bikesharing) {
  using namespace motis::reliability::intermodal::bikesharing;
  if (req.dep_is_intermodal()) {
    auto infos = retrieve_bikesharing_infos(true, req, max_duration,
                                            pareto_filtering_for_bikesharing);
    for (auto& info : infos) {
      auto const& i = info;
      if (i.station_eva_ == "8000105") {
        printf("\nREL %s %d+%dmin %f,%f --> %f,%f", i.station_eva_.c_str(),
               i.bike_duration_, i.walk_duration_, i.from_.lat_, i.from_.lng_,
               i.to_.lat_, i.to_.lng_);
      }
      bikesharing_at_start_.emplace_back(next_id(), std::move(info));
    }
  }
  if (req.arr_is_intermodal()) {
    auto infos = retrieve_bikesharing_infos(false, req, max_duration,
                                            pareto_filtering_for_bikesharing);
    for (auto& info : infos) {
      auto const& i = info;
      if (i.station_eva_ == "0657967") {
        printf("\nREL %s %d+%dmin %f,%f --> %f,%f", i.station_eva_.c_str(),
               i.bike_duration_, i.walk_duration_, i.from_.lat_, i.from_.lng_,
               i.to_.lat_, i.to_.lng_);
      }
      bikesharing_at_destination_.emplace_back(next_id(), std::move(info));
    }
  }
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
