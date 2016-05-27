#pragma once

#include <vector>

#include "motis/reliability/error.h"
#include "motis/reliability/intermodal/reliable_bikesharing.h"

namespace motis {
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
namespace intermodal {

struct individual_modes_container {
  explicit individual_modes_container(ReliableRoutingRequest const& req) {
    if (req.individual_modes()->bikesharing() == 1) {
      bikesharing_.init(req);
    }
    if (req.individual_modes()->taxi() == 1) {
      throw std::system_error(error::not_implemented);
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
};

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
