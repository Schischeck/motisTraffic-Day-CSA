#include "motis/reliability/search/connection_graph_builder.h"

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

  unsigned int stop_idx = 0;
  for (auto const& j : journeys) {
    cg.journeys.emplace_back();
    cg.journeys.back().j = j;
    cg.journeys.back().from_index = stop_idx;
    if (stop_idx == connection_graph::stop::Index_departure_stop &&
        journeys.size() > 1) {
      stop_idx = 1;
    }
    cg.journeys.back().to_index = ++stop_idx;
  }
  cg.journeys.back().to_index = connection_graph::stop::Index_arrival_stop;

  /* departure stop of cg */
  cg.stops.emplace_back();
  cg.stops.back().index = 0;
  cg.stops.back().eva_no = base_journey.stops.front().eva_no;
  cg.stops.back().name = base_journey.stops.front().name;
  /* arrival stop of cg */
  cg.stops.emplace_back();
  cg.stops.back().index = 1;
  cg.stops.back().eva_no = base_journey.stops.back().eva_no;
  cg.stops.back().name = base_journey.stops.back().name;

  for (unsigned int i = 0; i + 1 < cg.journeys.size(); ++i) {
    auto& j = cg.journeys[i];
    cg.stops.emplace_back();
    auto& stop = cg.stops.back();
    stop.index = j.to_index;
    stop.eva_no = j.j.stops.back().eva_no;
    stop.name = j.j.stops.back().name;
    stop.interchange_infos.emplace_back();
    stop.interchange_infos.back().departing_journey_index = i + 1;
  }
}

void add_alternative_journey(connection_graph&, unsigned int const stop_idx,
                             journey const&) {}

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
