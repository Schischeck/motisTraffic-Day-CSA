#include "motis/reliability/db_distributions.h"

#include <algorithm>
#include <cassert>

namespace motis {
namespace reliability {

db_distributions::db_distributions(std::string const root) : empty_string_("") {
  std::vector<db_distributions_loader::distribution_mapping<
      unsigned int const> > distribution_mappings;
  db_distributions_loader::load_distributions(
      root, family_to_distribution_class_, all_probability_distributions_,
      distribution_mappings, class_to_probability_distributions_);

  // convert distribution_mappings (vector)
  // to distribution_mappings_ (map containing vectors)
  for (auto const& orig_mapping : distribution_mappings) {
    auto distribution_it = std::find_if(
        all_probability_distributions_.begin(),
        all_probability_distributions_.end(),
        [orig_mapping](
            std::pair<unsigned int, probability_distribution> const& entry) {
          return entry.first == orig_mapping.distribution_;
        });
    assert(distribution_it != all_probability_distributions_.end());
    db_distributions_loader::distribution_mapping<
        probability_distribution const&>
        mapping(distribution_it->second, orig_mapping.distribution_class_,
                orig_mapping.travel_time_from_, orig_mapping.travel_time_to_,
                orig_mapping.delay_from_, orig_mapping.delay_to_);
    distribution_mappings_[orig_mapping.distribution_class_].push_back(mapping);
  }

  default_start_distribution_.init_one_point(0, 1.0);
}

std::string const& db_distributions::get_distribution_class(
    std::string const& family) const {
  auto const it = family_to_distribution_class_.find(family);
  if (it != family_to_distribution_class_.end()) {
    return it->second;
  }
  return empty_string_;
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

void db_distributions::get_travel_time_distributions(
    std::string const& family, unsigned int const travel_time,
    std::vector<travel_time_distribution>& distributions) const {
  std::string const& distribution_class = get_distribution_class(family);
  if (distribution_class != empty_string_) {
    auto const mappings_vector_it =
        distribution_mappings_.find(distribution_class);
    if (mappings_vector_it != distribution_mappings_.end()) {
      auto const& mappings_vector = mappings_vector_it->second;
      auto mapping_it = std::find_if(
          mappings_vector.begin(), mappings_vector.end(),
          [travel_time](db_distributions_loader::distribution_mapping<
              probability_distribution const&> const& m) {
            return m.travel_time_from_ <= travel_time &&
                   m.travel_time_to_ >= travel_time;
          });
      while (mapping_it != mappings_vector.end() &&
             mapping_it->travel_time_from_ <= travel_time &&
             mapping_it->travel_time_to_ >= travel_time) {
        distributions.emplace_back(mapping_it->distribution_,
                                   mapping_it->delay_from_,
                                   mapping_it->delay_to_);
        mapping_it++;
      }
    }
  }
}

}  // namespace reliability
}  // namespace motis
