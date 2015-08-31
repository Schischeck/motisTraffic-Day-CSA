#include "motis/reliability/db_distributions.h"

#include <algorithm>
#include <cassert>

namespace motis {
namespace reliability {

db_distributions::db_distributions(
    std::string const root, unsigned int const max_expected_travel_time,
    unsigned int const max_expected_departure_delay)
    : empty_string_("") {
  std::vector<db_distributions_loader::resolved_mapping> distribution_mappings;
  db_distributions_loader::load_distributions(
      root, max_expected_travel_time, max_expected_departure_delay,
      family_to_distribution_class_, all_probability_distributions_,
      distribution_mappings, class_to_probability_distributions_);

  // convert distribution_mappings (vector)
  // to distribution_mappings_ (map containing vectors)
  for (auto const& orig_mapping : distribution_mappings) {
    auto distribution_it = std::find_if(
        all_probability_distributions_.begin(),
        all_probability_distributions_.end(),
        [orig_mapping](
            std::pair<unsigned int, probability_distribution> const& entry) {
          return entry.first ==
                 std::get<db_distributions_loader::resolved_mapping_pos::
                              rm_distribution_id>(orig_mapping);
        });
    assert(distribution_it != all_probability_distributions_.end());
    distribution_mapping mapping(
        distribution_it->second,
        std::get<db_distributions_loader::resolved_mapping_pos::rm_travel_time>(
            orig_mapping),
        std::get<db_distributions_loader::resolved_mapping_pos::rm_delay>(
            orig_mapping));
    distribution_mappings_
        [std::get<db_distributions_loader::resolved_mapping_pos::rm_class>(
             orig_mapping)].push_back(mapping);
  }

  default_start_distribution_.init_one_point(0, 1.0);
  default_travel_time_distribution_.init_one_point(0, 1.0);
}

void db_distributions::get_travel_time_distributions(
    std::string const& family, unsigned int const travel_time,
    unsigned int const to_departure_delay,
    std::vector<probability_distribution_cref>& distributions) const {
  std::string const& distribution_class = get_distribution_class(family);
  if (distribution_class != empty_string_) {
    auto const mappings_vector_it =
        distribution_mappings_.find(distribution_class);
    if (mappings_vector_it != distribution_mappings_.end()) {
      get_distributions(travel_time, to_departure_delay,
                        mappings_vector_it->second, distributions);
    }
  }
}

probability_distribution const& db_distributions::get_start_distribution(
    std::string const& family) const {
  std::string const& distribution_class = get_distribution_class(family);
  if (distribution_class != empty_string_) {
    auto const it =
        class_to_probability_distributions_.find(distribution_class);
    if (it != class_to_probability_distributions_.end()) {
      return it->second;
    }
  }
  return default_start_distribution_;
}

std::string const& db_distributions::get_distribution_class(
    std::string const& family) const {
  auto const it = family_to_distribution_class_.find(family);
  if (it != family_to_distribution_class_.end()) {
    return it->second;
  }
  return empty_string_;
}

void db_distributions::get_distributions(
    unsigned int const travel_time, unsigned int const to_departure_delay,
    std::vector<db_distributions::distribution_mapping> const& all_mappings,
    std::vector<start_and_travel_distributions::probability_distribution_cref>&
        distributions) const {
  auto const mapping_begin =
      std::lower_bound(all_mappings.begin(), all_mappings.end(), travel_time,
                       [](db_distributions::distribution_mapping const& a,
                          unsigned int const b) { return a.travel_time_ < b; });
  // if there are no mappings for 'travel_time'
  // deliver the mappings for the largest travel time
  if (mapping_begin == all_mappings.end()) {
    get_distributions(all_mappings.back().travel_time_, to_departure_delay,
                      all_mappings, distributions);
    return;
  }

  auto mapping_end =
      std::upper_bound(mapping_begin, all_mappings.end(), travel_time,
                       [](unsigned int const a,
                          db_distributions::distribution_mapping const& b) {
                         return a < b.travel_time_;
                       });
  --mapping_end;

  if (std::distance(mapping_begin, mapping_end) < 0 && travel_time == 0) {
    for (unsigned int i = 0; i <= to_departure_delay; i++) {
      distributions.push_back(std::ref(default_travel_time_distribution_));
    }
    return;
  }

  assert(std::distance(mapping_begin, mapping_end) >= 0);
  assert(mapping_begin->travel_time_ == travel_time);
  assert(mapping_end->travel_time_ == travel_time);

  for (auto it = mapping_begin;
       it != mapping_end && it->delay_ <= to_departure_delay; it++) {
    distributions.push_back(std::ref(it->distribution_));
  }

  // If distributions for departure delays are required
  // which do not exist in the db-distributions-files,
  // for those delays, return the distribution for the
  // latest departure delay that exists in the db-distributions-files.
  while (distributions.size() <= to_departure_delay) {
    distributions.push_back(distributions.back());
  }
  assert(distributions.size() == to_departure_delay + 1);
}

}  // namespace reliability
}  // namespace motis
