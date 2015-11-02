#include "motis/reliability/search/connection_graph_builder.h"

#include <cassert>

#include "motis/core/common/journey.h"
#include "motis/core/common/journey_builder.h"

#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_builder {
namespace detail {
/* split journey at a stop with interchange */
std::pair<journey, journey> split_journey(journey const& j,
                                          unsigned int const stop_idx) {
  assert(j.stops.at(stop_idx).interchange);
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

  for (auto const& transport : j.transports) {
    if (transport.to <= stop_idx) {
      j1.transports.push_back(transport);
    } else if (transport.from >= stop_idx) {
      j2.transports.push_back(transport);
      j2.transports.back().from -= stop_idx;
      j2.transports.back().to -= stop_idx;
    }
  }
  for (auto const& attribute : j.attributes) {
    if (attribute.from < stop_idx) {
      j1.attributes.push_back(attribute);
      j1.attributes.back().to =
          std::min((unsigned int)j1.attributes.back().to, stop_idx);
    }
    if (attribute.to > stop_idx) {
      j2.attributes.push_back(attribute);
      j2.attributes.back().from =
          std::max((unsigned int)j2.attributes.back().from, stop_idx) -
          stop_idx;
      j2.attributes.back().to -= stop_idx;
    }
  }

  j1.duration = journey_builder::detail::get_duration(j1);
  j2.duration = journey_builder::detail::get_duration(j2);
  j1.transfers = journey_builder::detail::get_transfers(j1);
  j2.transfers = journey_builder::detail::get_transfers(j2);
  j1.price = 0;
  j2.price = 0;

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
}  // namespace detail

void add_base_journey(connection_graph& cg, journey const& base_journey) {
  auto journeys = split_journey(base_journey);

  unsigned int stop_idx = connection_graph::stop::Index_departure_stop;
  for (auto const& j : journeys) {
    cg.stops.emplace_back();
    auto& stop = cg.stops.back();
    stop.index = stop_idx;
    stop.departure_infos.emplace_back();
    auto& departure_info = stop.departure_infos.front();
    departure_info.departing_journey_index = cg.journeys.size();
    if (stop_idx == connection_graph::stop::Index_departure_stop &&
        journeys.size() > 1) {
      stop_idx = connection_graph::stop::Index_first_intermediate_stop;
    } else if (departure_info.departing_journey_index + 1 == journeys.size()) {
      stop_idx = connection_graph::stop::Index_arrival_stop;
    } else {
      ++stop_idx;
    }
    departure_info.head_stop_index = stop_idx;

    if (cg.stops.size() == 1) {
      cg.stops.emplace_back();
      cg.stops.back().index = connection_graph::stop::Index_arrival_stop;
    }

    cg.journeys.emplace_back();
    cg.journeys.back().j = j;
  }
}

connection_graph::stop& get_stop(connection_graph& cg,
                                 unsigned int const first_stop_idx,
                                 unsigned int const stop_idx) {
  if (stop_idx == first_stop_idx) {
    return cg.stops.at(first_stop_idx);
  }
  cg.stops.emplace_back();
  cg.stops.back().index = stop_idx;
  return cg.stops.back();
}
void add_alternative_journey(connection_graph& cg,
                             unsigned int const first_stop_idx,
                             journey const& j) {
  auto journeys = split_journey(j);

  unsigned int stop_idx = first_stop_idx, journey_count = 0;
  for (auto const& j : journeys) {
    auto& stop = get_stop(cg, first_stop_idx, stop_idx);
    stop.departure_infos.emplace_back();
    auto& departure_info = stop.departure_infos.back();
    departure_info.departing_journey_index = cg.journeys.size();
    if (journey_count + 1 == journeys.size()) {
      stop_idx = connection_graph::stop::Index_arrival_stop;
    } else if (stop_idx == first_stop_idx) {
      stop_idx = cg.stops.size();
    } else {
      ++stop_idx;
    }
    departure_info.head_stop_index = stop_idx;

    cg.journeys.emplace_back();
    cg.journeys.back().j = j;
    ++journey_count;
  }
}

std::vector<journey> split_journey(journey const& j) {
  std::vector<journey> journeys;
  journeys.push_back(j);
  detail::split_journey(journeys);
  return journeys;
}

}  // namespace connection_graph_builder
}  // namespace search
}  // namespace reliability
}  // namespace motis
