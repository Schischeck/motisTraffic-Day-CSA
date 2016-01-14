#include "motis/reliability/search/connection_graph_builder.h"

#include <cassert>

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
    if (element.from < stop_idx) {
      j1.push_back(element);
      j1.back().to = std::min((unsigned int)j1.back().to, stop_idx);
    }
    if (element.to > stop_idx) {
      j2.push_back(element);
      j2.back().from =
          std::max((unsigned int)j2.back().from, stop_idx) - stop_idx;
      j2.back().to -= stop_idx;
    }
  }
}

/* split journey at a stop with interchange */
std::pair<journey, journey> split_journey(journey const& j,
                                          unsigned int const stop_idx) {
  assert(j.transports.size() >= 1);
  assert(j.stops.size() > 2);
  assert(j.stops.at(stop_idx).interchange);
  assert(stop_idx > 0);
  assert(stop_idx + 1 < j.stops.size());
  journey j1;
  journey j2;

  for (auto const& stop : j.stops) {
    if (stop.index <= stop_idx) {
      j1.stops.push_back(stop);
    }
    if (stop.index >= stop_idx) {
      j2.stops.push_back(stop);
      j2.stops.back().index -= stop_idx;
    }
  }
  j1.stops.back().interchange = false;
  j1.stops.back().departure.valid = false;
  j2.stops.front().interchange = false;
  j2.stops.front().arrival.valid = false;

  correct_transports_and_attributes_indices(j.transports, j1.transports,
                                            j2.transports, stop_idx);
  correct_transports_and_attributes_indices(j.attributes, j1.attributes,
                                            j2.attributes, stop_idx);

  j1.duration = get_duration(j1);
  j2.duration = get_duration(j2);
  j1.transfers = get_transfers(j1);
  j2.transfers = get_transfers(j2);
  j1.price = 0;
  j2.price = 0;
  j1.night_penalty = 0;
  j2.night_penalty = 0;

  assert(j1.transports.size() >= 1);
  assert(j1.stops.size() >= 2);
  assert(j2.transports.size() >= 1);
  assert(j2.stops.size() >= 2);
  assert(j1.stops.size() + j2.stops.size() == j.stops.size() + 1);
  assert(!j1.stops.front().interchange);
  assert(!j1.stops.back().interchange);
  assert(!j2.stops.front().interchange);
  assert(!j2.stops.back().interchange);

  return std::make_pair(j1, j2);
}

void split_journey(std::vector<journey>& journeys) {
  auto const& stop =
      std::find_if(journeys.back().stops.begin(), journeys.back().stops.end(),
                   [](journey::stop const& s) { return s.interchange; });
  if (stop == journeys.back().stops.end()) {
    return;
  }
  auto splitted_journey = split_journey(journeys.back(), stop->index);
  journeys.pop_back();
  journeys.push_back(splitted_journey.first);
  journeys.push_back(splitted_journey.second);
  split_journey(journeys);
}

journey remove_dummy_stops(journey const& orig_journey) {
  assert(orig_journey.transports.size() >= 1);
  assert(orig_journey.stops.size() >= 2);
  journey j = orig_journey;
  if (j.stops.front().name == "DUMMY") {
    j.stops.erase(j.stops.begin());
    j.stops.front().arrival.valid = false;
    j.transports.erase(j.transports.begin());
    for (auto& t : j.transports) {
      --t.from;
      --t.to;
    }
    for (auto& a : j.attributes) {
      --a.from;
      --a.to;
    }
    for (auto& stop : j.stops) {
      --stop.index;
    }
  }
  if (j.stops.back().name == "DUMMY") {
    j.stops.pop_back();
    j.stops.back().departure.valid = false;
    j.stops.back().interchange = false;
    j.transports.pop_back();
  }
  assert(j.transports.size() >= 1);
  assert(j.stops.size() >= 2);
  return j;
}

std::vector<journey> split_journey(journey const& orig_journey) {
  std::vector<journey> journeys;
  journeys.push_back(orig_journey);
  detail::split_journey(journeys);
  return journeys;
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
  auto journeys =
      detail::split_journey(detail::remove_dummy_stops(base_journey));

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
               (unsigned short)journeys.size()) {
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
  auto journeys = detail::split_journey(detail::remove_dummy_stops(j));

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
