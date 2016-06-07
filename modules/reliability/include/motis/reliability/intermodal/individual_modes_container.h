#pragma once

#include <string>
#include <vector>

#include "motis/core/journey/journey.h"

#include "motis/reliability/error.h"
#include "motis/reliability/intermodal/hotels.h"
#include "motis/reliability/intermodal/reliable_bikesharing.h"

namespace motis {
struct journey;
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
namespace intermodal {

enum mode_type { WALK, BIKESHARING, TAXI, HOTEL };

constexpr auto LATE_TAXI_BEGIN_TIME = 1140;  // 19:00 GMT
constexpr auto LATE_TAXI_END_TIME = 180;  // 03:00 GMT

struct individual_modes_container {
  /* for late connections */
  individual_modes_container() = default;

  /* for bikesharing requests */
  individual_modes_container(ReliableRoutingRequest const& req,
                             unsigned const max_bikesharing_duration) {
    if (req.individual_modes()->bikesharing() == 1) {
      bikesharing_.init(req, max_bikesharing_duration);
    }
  }

  int get_id_offset(mode_type const t) const {
    switch (t) {
      case BIKESHARING: return 0;
      case TAXI:
        return bikesharing_.at_start_.size() +
               bikesharing_.at_destination_.size();
      case HOTEL: return taxis_.size() + get_id_offset(TAXI);
      default: break;
    }
    throw std::system_error(error::failure);
  }

  mode_type get_mumo_type(int const id) const {
    if (id == -1) {
      return WALK;
    }
    if (id < get_id_offset(TAXI)) {
      return BIKESHARING;
    }
    if (id < get_id_offset(HOTEL)) {
      return TAXI;
    }
    return HOTEL;
    throw std::system_error(error::failure);
  }

  struct bikesharing {
    void init(ReliableRoutingRequest const& req, unsigned const max_duration) {
      using namespace motis::reliability::intermodal::bikesharing;
      if (req.dep_is_intermodal()) {
        at_start_ = retrieve_bikesharing_infos(true, req, max_duration);
      }
      if (req.arr_is_intermodal()) {
        at_destination_ = retrieve_bikesharing_infos(false, req, max_duration);
      }
    }

    using bs_type =
        motis::reliability::intermodal::bikesharing::bikesharing_info;
    std::vector<bs_type> at_start_, at_destination_;
  } bikesharing_;

  std::vector<hotel> hotels_;

  struct taxi {
    explicit taxi(std::string const from_station, std::string const to_station,
                  uint16_t const duration, uint16_t const price,
                  uint16_t const valid_from = LATE_TAXI_BEGIN_TIME,
                  uint16_t const valid_to = LATE_TAXI_END_TIME)
        : from_station_(from_station),
          to_station_(to_station),
          duration_(duration),
          price_(price),
          valid_from_(valid_from),
          valid_to_(valid_to) {}

    std::string from_station_, to_station_;
    uint16_t duration_;
    uint16_t price_;
    uint16_t valid_from_, valid_to_;
  };
  std::vector<taxi> taxis_;
};

inline std::string to_str(mode_type const t) {
  switch (t) {
    case WALK: return "Walk";
    case BIKESHARING: return "Bikesharing";
    case TAXI: return "Taxi";
    case HOTEL: return "Hotel";
  }
  return "unknown";
}

inline void update_mumo_info(journey& j,
                             individual_modes_container const& container) {
  for (auto& t : j.transports_) {
    t.mumo_type_ = intermodal::to_str(container.get_mumo_type(t.mumo_id_));
  }
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
