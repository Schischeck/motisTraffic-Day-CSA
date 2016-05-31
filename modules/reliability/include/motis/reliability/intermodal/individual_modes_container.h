#pragma once

#include <string>
#include <vector>

#include "motis/reliability/error.h"
#include "motis/reliability/intermodal/reliable_bikesharing.h"

namespace motis {
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
namespace intermodal {

enum slot { WALK = 0, BIKESHARING = 1, TAXI = 2, HOTEL = 3 };

inline std::string to_str(slot const s) {
  switch (s) {
    case WALK: return "Walk";
    case BIKESHARING: return "Bikesharing";
    case TAXI: return "Taxi";
    case HOTEL: return "Hotel";
  }
  return "unknown";
}

constexpr auto HOTEL_EARLIEST_CHECKOUT = 8 * 60;
constexpr auto HOTEL_MIN_STAY_DURATION = 9 * 60;
constexpr auto HOTEL_PRICE = 5000;

constexpr auto LATE_TAXI_BEGIN_TIME = 1260;  // minutes after midnight
constexpr auto LATE_TAXI_END_TIME = 180;  // minutes after midnight

struct individual_modes_container {
  /* for late connections */
  individual_modes_container() = default;

  /* for bikesharing requests */
  explicit individual_modes_container(ReliableRoutingRequest const& req) {
    if (req.individual_modes()->bikesharing() == 1) {
      bikesharing_.init(req);
    }
  }

  struct bikesharing {
    void init(ReliableRoutingRequest const& req) {
      using namespace motis::reliability::intermodal::bikesharing;
      if (req.dep_is_intermodal()) {
        at_start_ = retrieve_bikesharing_infos(true, req);
      }
      if (req.arr_is_intermodal()) {
        at_destination_ = retrieve_bikesharing_infos(false, req);
      }
    }

    using bs_type =
        motis::reliability::intermodal::bikesharing::bikesharing_info;
    std::vector<bs_type> at_start_, at_destination_;
  } bikesharing_;

  struct hotel {
    hotel(std::string const station,
          uint16_t const earliest_checkout = HOTEL_EARLIEST_CHECKOUT,
          uint16_t const min_stay_duration = HOTEL_MIN_STAY_DURATION,
          uint16_t const price = HOTEL_PRICE)
        : station_(station),
          earliest_checkout_(earliest_checkout),
          min_stay_duration_(min_stay_duration),
          price_(price) {}

    std::string station_;
    uint16_t earliest_checkout_;
    uint16_t min_stay_duration_;
    uint16_t price_;
  };
  std::vector<hotel> hotel_;

  struct taxi {
    taxi(std::string const from_station, std::string const to_station,
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
  std::vector<taxi> taxi_;
};

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
