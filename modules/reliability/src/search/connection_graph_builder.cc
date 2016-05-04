#include "motis/reliability/search/connection_graph_builder.h"

#include <cassert>

#include "motis/core/common/logging.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_builder {
namespace detail {

template <typename T>
void correct_transports_and_attributes_indices(std::vector<T> const& orig,
                                               std::vector<T>& j1,
                                               std::vector<T>& j2,
                                               unsigned int const stop_idx) {
  for (auto const& element : orig) {
    if (element.from_ < stop_idx) {
      j1.push_back(element);
      j1.back().to_ = std::min((unsigned int)j1.back().to_, stop_idx);
    }
    if (element.to_ > stop_idx) {
      j2.push_back(element);
      j2.back().from_ =
          std::max((unsigned int)j2.back().from_, stop_idx) - stop_idx;
      j2.back().to_ -= stop_idx;
    }
  }
}

/* split journey at a stop with interchange */
std::pair<journey, journey> split_journey(journey const& j,
                                          unsigned int const stop_idx) {
  assert(!j.transports_.empty());
  assert(j.stops_.size() > 2);
  assert(j.stops_.at(stop_idx).interchange_);
  assert(stop_idx > 0);
  assert(stop_idx + 1 < j.stops_.size());

  journey j1;
  journey j2;

  for (unsigned idx = 0; idx < j.stops_.size(); ++idx) {
    if (idx <= stop_idx) {
      j1.stops_.push_back(j.stops_[idx]);
    }
    if (idx >= stop_idx) {
      j2.stops_.push_back(j.stops_[idx]);
    }
  }
  j1.stops_.back().interchange_ = false;
  j1.stops_.back().departure_.valid_ = false;
  j2.stops_.front().interchange_ = false;
  j2.stops_.front().arrival_.valid_ = false;

  correct_transports_and_attributes_indices(j.transports_, j1.transports_,
                                            j2.transports_, stop_idx);
  correct_transports_and_attributes_indices(j.attributes_, j1.attributes_,
                                            j2.attributes_, stop_idx);

  j1.duration_ = get_duration(j1);
  j2.duration_ = get_duration(j2);
  j1.transfers_ = get_transfers(j1);
  j2.transfers_ = get_transfers(j2);
  j1.price_ = 0;
  j2.price_ = 0;
  // j1.night_penalty = 0; // TODO
  // j2.night_penalty = 0; // TODO

  assert(!j1.transports_.empty());
  assert(j1.stops_.size() >= 2);
  assert(!j2.transports_.empty());
  assert(j2.stops_.size() >= 2);
  assert(j1.stops_.size() + j2.stops_.size() == j.stops_.size() + 1);
  assert(!j1.stops_.front().interchange_);
  assert(!j1.stops_.back().interchange_);
  assert(!j2.stops_.front().interchange_);
  assert(!j2.stops_.back().interchange_);

  return std::make_pair(j1, j2);
}

bool no_public_transport_after_this_stop(journey const& j,
                                         unsigned int const stop_idx) {
  auto const& transport_it =
      std::find_if(j.transports_.begin(), j.transports_.end(),
                   [stop_idx](journey::transport const& t) {
                     return t.from_ <= stop_idx && t.to_ > stop_idx;
                   });
  if (transport_it == j.transports_.end()) {
    LOG(logging::error) << "Transport not found!";
    return true;
  }

  auto const& public_transport_it = std::find_if(
      transport_it, j.transports_.end(), [](journey::transport const& t) {
        return t.type_ == journey::transport::PublicTransport;
      });

  return public_transport_it == j.transports_.end();
}

void split_journey(std::vector<journey>& journeys) {
  auto const& stop =
      std::find_if(journeys.back().stops_.begin(), journeys.back().stops_.end(),
                   [](journey::stop const& s) { return s.interchange_; });
  auto const stop_idx = std::distance(journeys.back().stops_.begin(), stop);
  if (stop == journeys.back().stops_.end() ||
      no_public_transport_after_this_stop(journeys.back(), stop_idx)) {
    return;
  }
  auto splitted_journey = split_journey(journeys.back(), stop_idx);
  journeys.pop_back();
  journeys.push_back(splitted_journey.first);
  journeys.push_back(splitted_journey.second);
  split_journey(journeys);
}

journey remove_dummy_stops(journey const& orig_journey) {
  assert(!orig_journey.transports_.empty());
  assert(orig_journey.stops_.size() >= 2);
  journey j = orig_journey;
  if (j.stops_.front().name_ == "DUMMY") {
    j.stops_.erase(j.stops_.begin());
    j.stops_.front().arrival_.valid_ = false;
    j.transports_.erase(j.transports_.begin());
    for (auto& t : j.transports_) {
      --t.from_;
      --t.to_;
    }
    for (auto& a : j.attributes_) {
      --a.from_;
      --a.to_;
    }
  }
  if (j.stops_.back().name_ == "DUMMY") {
    j.stops_.pop_back();
    j.stops_.back().departure_.valid_ = false;
    j.stops_.back().interchange_ = false;
    j.transports_.pop_back();
  }
  assert(!j.transports_.empty());
  assert(j.stops_.size() >= 2);
  return j;
}

std::vector<journey> split_journey(journey const& orig_journey) {
  std::vector<journey> journeys;
  journeys.push_back(orig_journey);
  detail::split_journey(journeys);
  return journeys;
}

journey move_early_walk(journey const& orig_j) {
  journey j = orig_j;
  if (j.transports_.size() > 1 && j.stops_.size() > 2) {
    unsigned int const index_last_stop = j.stops_.size() - 1;
    for (auto const& tr : j.transports_) {
      if (tr.type_ == journey::transport::Walk && tr.to_ < index_last_stop) {
        auto const buffer = j.stops_[tr.to_].departure_.timestamp_ -
                            j.stops_[tr.to_].arrival_.timestamp_;
        if (buffer > 0) {
          j.stops_[tr.from_].departure_.timestamp_ += buffer;
          j.stops_[tr.from_].departure_.schedule_timestamp_ += buffer;
          j.stops_[tr.to_].arrival_.timestamp_ += buffer;
          j.stops_[tr.to_].arrival_.schedule_timestamp_ += buffer;
        }
      }
    }
  }
  return j;
}

connection_graph::stop& get_stop(connection_graph& cg,
                                 unsigned int const first_stop_idx,
                                 unsigned int const stop_idx) {
  if (stop_idx == first_stop_idx) {
    return cg.stops_.at(first_stop_idx);
  }
  cg.stops_.emplace_back();
  cg.stops_.back().index_ = stop_idx;
  return cg.stops_.back();
}

}  // namespace detail

void add_base_journey(connection_graph& cg, journey const& base_journey) {
  auto journeys = detail::split_journey(
      detail::move_early_walk(detail::remove_dummy_stops(base_journey)));

  unsigned int stop_idx = connection_graph::stop::Index_departure_stop;
  for (auto const& j : journeys) {
    cg.stops_.emplace_back();
    auto& stop = cg.stops_.back();
    stop.index_ = stop_idx;
    stop.alternative_infos_.emplace_back();
    auto& alternative_info = stop.alternative_infos_.front();
    alternative_info.journey_index_ = cg.journeys_.size();
    if (stop_idx == connection_graph::stop::Index_departure_stop &&
        journeys.size() > 1) {
      stop_idx = connection_graph::stop::Index_first_intermediate_stop;
    } else if (alternative_info.journey_index_ + 1 ==
               (uint16_t)journeys.size()) {
      stop_idx = connection_graph::stop::Index_arrival_stop;
    } else {
      ++stop_idx;
    }
    alternative_info.next_stop_index_ = stop_idx;

    if (cg.stops_.size() == 1) {
      cg.stops_.emplace_back();
      cg.stops_.back().index_ = connection_graph::stop::Index_arrival_stop;
    }

    cg.journeys_.push_back(j);
  }
}

void add_alternative_journey(connection_graph& cg,
                             unsigned int const first_stop_idx,
                             journey const& j) {
  auto journeys = detail::split_journey(
      detail::move_early_walk(detail::remove_dummy_stops(j)));

  /* todo:
   * call function: std::vector<unsigned int> add_journeys(cg, journeys);
   * for each journey, this function check whether it is already contained in
   * cg.journeys. Only if not, it adds the journey to cg.journeys.
   * The indices of the journeys (either added or found in cg.journeys) are
   * delivered and used in the following for-loop.
   * (The for-loop would not touch cg.journeys anymore) */

  unsigned int stop_idx = first_stop_idx, journey_count = 0;
  for (auto const& j : journeys) {
    auto& stop = detail::get_stop(cg, first_stop_idx, stop_idx);
    stop.alternative_infos_.emplace_back();
    auto& departure_info = stop.alternative_infos_.back();
    departure_info.journey_index_ = cg.journeys_.size();
    if (journey_count + 1 == journeys.size()) {
      stop_idx = connection_graph::stop::Index_arrival_stop;
    } else if (stop_idx == first_stop_idx) {
      stop_idx = cg.stops_.size();
    } else {
      ++stop_idx;
    }
    departure_info.next_stop_index_ = stop_idx;

    cg.journeys_.push_back(j);
    ++journey_count;
  }
}

}  // namespace connection_graph_builder
}  // namespace search
}  // namespace reliability
}  // namespace motis
