#pragma once

#include <string>
#include <vector>

#include "motis/reliability/error.h"
#include "motis/reliability/intermodal/hotels.h"
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

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
