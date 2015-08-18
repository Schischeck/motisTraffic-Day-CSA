#pragma once

#include <map>
#include <string>

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

/**
 * Functions to read CSV-files (with ';' separator) containing
 * distributions for travel-times and start events of trains.
 * Note: the files have to be in Unix-format without carriage return
 * and usage of "Umlaut" in the headers is not allowed.
 * Float-values have to use '.' as decimal mark.
 */
namespace db_distributions_loader {

using mapping_csv = std::tuple<int, std::string, int, int, int, int>;

struct distribution_mapping {
  distribution_mapping(unsigned int const distribution_id,
                       std::string const& distribution_class,
                       unsigned int const travel_time_from,
                       unsigned int const travel_time_to, int const delay_from,
                       int const delay_to)
      : distribution_id_(distribution_id),
        distribution_class_(distribution_class),
        travel_time_from_(travel_time_from),
        travel_time_to_(travel_time_to),
        delay_from_(delay_from),
        delay_to_(delay_to) {}
  unsigned int const distribution_id_;
  std::string distribution_class_;
  unsigned int const travel_time_from_;
  unsigned int const travel_time_to_;
  int const delay_from_;
  int const delay_to_;
};

void load_distributions(std::string const root);

namespace detail {
void load_distributions_classes(
    std::string const filepath,
    std::map<std::string, std::string>& family_to_distribution_class);
void load_distributions(
    std::string const filepath,
    std::vector<std::pair<unsigned int, probability_distribution> >&
        probability_distributions);
void load_distribution_mappings(
    std::string const filepath,
    std::vector<distribution_mapping>& distribution_mappings);
void load_start_distributions(std::string const filepath,
                              std::map<std::string, probability_distribution>&
                                  class_to_probability_distributions);

bool mapping_is_smaller(mapping_csv a, mapping_csv b);

}  // namespace detail
}  // namespace distributions_loader

}  // namespace reliability
}  // namespace motis
