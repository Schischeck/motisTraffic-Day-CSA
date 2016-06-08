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
  individual_modes_container() : id_counter_(0) {}

  /* for bikesharing requests */
  individual_modes_container(ReliableRoutingRequest const& req,
                             unsigned const max_bikesharing_duration)
      : id_counter_(0) {
    if (req.individual_modes()->bikesharing() == 1) {
      init_bikesharing(req, max_bikesharing_duration);
    }
  }

  int next_id() { return id_counter_++; }

  mode_type get_mumo_type(int const id) const;

  void init_bikesharing(ReliableRoutingRequest const& req,
                        unsigned const max_duration) {
    using namespace motis::reliability::intermodal::bikesharing;
    if (req.dep_is_intermodal()) {
      auto infos = retrieve_bikesharing_infos(true, req, max_duration);
      for (auto& info : infos) {
        bikesharing_at_start_.emplace_back(next_id(), std::move(info));
      }
    }
    if (req.arr_is_intermodal()) {
      auto infos = retrieve_bikesharing_infos(false, req, max_duration);
      for (auto& info : infos) {
        bikesharing_at_destination_.emplace_back(next_id(), std::move(info));
      }
    }
  }

  void insert_hotel(intermodal::hotel const& h) {
    hotels_.emplace_back(next_id(), h);
  }

  std::vector<std::pair<int, bikesharing::bikesharing_info>>
      bikesharing_at_start_, bikesharing_at_destination_;

  std::vector<std::pair<int, hotel>> hotels_;

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
  std::vector<std::pair<int, taxi>> taxis_;

  void insert_taxi(taxi const& t) { taxis_.emplace_back(next_id(), t); }

  int id_counter_;
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
    if (t.is_walk_ && t.mumo_id_ >= 0) {
      t.mumo_type_ = intermodal::to_str(container.get_mumo_type(t.mumo_id_));
    }
  }
}

motis::reliability::intermodal::bikesharing::bikesharing_info const&
get_bikesharing_info(individual_modes_container const&, int const mumo_id);

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
