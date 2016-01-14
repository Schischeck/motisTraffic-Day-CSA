#include "motis/reliability/rating/cg_arrival_distribution.h"

#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace rating {
namespace cg {
namespace detail {

using distribution_info = std::pair<time_t, probability_distribution>;

std::vector<distribution_info> distributions_arriving_alternatives(
    search::connection_graph const& cg) {
  /* get the distributions of all arriving alternatives */
  std::vector<distribution_info> distributions;
  for (auto const& stop : cg.stops_) {
    for (auto const& alternative : stop.alternative_infos_) {
      if (alternative.next_stop_index_ ==
          search::connection_graph::stop::Index_arrival_stop) {
        std::vector<probability> values;
        alternative.rating_.arrival_distribution_.get_probabilities(values);
        probability_distribution pd;
        pd.init(values, 0);
        distributions.emplace_back(
            cg.journeys_.at(alternative.journey_index_)
                    .stops.back()
                    .arrival.schedule_timestamp +
                (alternative.rating_.arrival_distribution_.first_minute() * 60),
            pd);
      }
    }
  }
  assert(distributions.size() > 0);
  return distributions;
}

/* note: distributions are moved such that they start with delay minute 0 */
time_t earliest_arrival_time(
    std::vector<distribution_info> const& distributions) {
  auto const& min_element = *std::min_element(
      distributions.begin(), distributions.end(),
      [](distribution_info const& a, distribution_info const& b) {
        return a.first < b.first;
      });
  return min_element.first;
}
time_t latest_arrival_time(
    std::vector<distribution_info> const& distributions) {
  auto const& max_element = *std::max_element(
      distributions.begin(), distributions.end(),
      [](distribution_info const& a, distribution_info const& b) {
        return a.first + (a.second.last_minute() * 60) <
               b.first + (b.second.last_minute() * 60);
      });
  return max_element.first + (max_element.second.last_minute() * 60);
}
}  // namespace detail

std::pair<time_t, probability_distribution> calc_arrival_distribution(
    search::connection_graph const& cg) {
  auto const distributions = detail::distributions_arriving_alternatives(cg);
  time_t const begin_time = detail::earliest_arrival_time(distributions);
  time_t const end_time = detail::latest_arrival_time(distributions);
  unsigned int const num_values = ((end_time - begin_time) / 60) + 1;
  assert(end_time >= begin_time);
  assert(num_values > 0);

  std::vector<probability> values(num_values);
  std::fill(values.begin(), values.end(), 0.0);
  for (auto const& dist_info : distributions) {
    assert(dist_info.first >= begin_time);
    unsigned int const offset = (dist_info.first - begin_time) / 60;
    auto const& pd = dist_info.second;
    for (int delay = pd.first_minute(); delay <= pd.last_minute(); ++delay) {
      assert((int)offset + delay >= 0);
      assert(offset + delay < values.size());
      values[offset + delay] += pd.probability_equal(delay);
    }
  }

  probability_distribution pd;
  pd.init(values, 0);
  return std::make_pair(begin_time, pd);
}

}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
