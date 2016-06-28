#pragma once

#include <map>
#include <string>

#include "motis/reliability/distributions/probability_distribution.h"

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
using resolved_mapping =
    std::tuple<std::string, unsigned int, unsigned int, unsigned int>;
enum resolved_mapping_pos {
  rm_class,
  rm_travel_time,
  rm_delay,
  rm_distribution_id
};

void load_distributions(
    std::string root, unsigned int const max_expected_travel_time,
    unsigned int const max_expected_departure_delay,
    std::map<std::string, std::string>& family_to_distribution_class,
    std::vector<std::pair<unsigned int, probability_distribution>>&
        probability_distributions,
    std::vector<resolved_mapping>& distribution_mappings,
    std::map<std::string, probability_distribution>&
        class_to_probability_distributions);

namespace detail {
using mapping_int = std::tuple<unsigned int, std::string, unsigned int,
                               unsigned int, unsigned int, unsigned int>;

void load_distributions_classes(
    std::string const filepath,
    std::map<std::string, std::string>& family_to_distribution_class);
void load_distributions(
    std::string const filepath,
    std::vector<std::pair<unsigned int, probability_distribution>>&
        probability_distributions);
void load_distribution_mappings(
    std::string const filepath, unsigned int const max_expected_travel_time,
    unsigned int const max_expected_departure_delay,
    std::vector<resolved_mapping>& resolved_mappings);
void load_start_distributions(std::string const filepath,
                              std::map<std::string, probability_distribution>&
                                  class_to_probability_distributions);

bool parse_travel_time_interval(std::string const& from_travel_time_str,
                                std::string const& to_travel_time_str,
                                unsigned int const max_expected_travel_time,
                                unsigned int& from_travel_time,
                                unsigned int& to_travel_time);
bool parse_departure_delay_interval(
    std::string const& from_delay_str, std::string const& to_delay_str,
    unsigned int const max_expected_departure_delay, unsigned int& from_delay,
    unsigned int& to_delay);

void resolve_mappings(std::vector<mapping_int> const& integer_mappings,
                      std::vector<resolved_mapping>& resolved_mappings);
bool mapping_is_smaller(resolved_mapping const& a, resolved_mapping const& b);

}  // namespace detail
}  // namespace db_distributions_loader

}  // namespace reliability
}  // namespace motis
