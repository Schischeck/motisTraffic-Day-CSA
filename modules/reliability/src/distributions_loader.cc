#include "motis/reliability/distributions_loader.h"

#include <utility>

namespace motis {
namespace reliability {
namespace distributions_loader {

void load_distributions(std::string const root) {}

namespace detail {

void load_distributions_classes(
    std::string const filepath,
    std::map<std::string, std::string>& family_to_distribution_class) {
  std::vector<classes_csv> classes_entries;
  parser::read_file<classes_csv, ';'>(filepath.c_str(), classes_entries,
                                      classes_columns);

  for (auto const& entry : classes_entries) {
    family_to_distribution_class[std::get<classes::c_family>(entry)] =
        std::get<classes::c_distribution_class>(entry);
  }
}

bool distribution_is_smaller(distributions_csv a, distributions_csv b) {
  if (std::get<distributions::d_distribution_id>(a) <
      std::get<distributions::d_distribution_id>(b))
    return true;
  return std::get<distributions::d_distribution_id>(a) ==
             std::get<distributions::d_distribution_id>(b) &&
         std::get<distributions::d_delay_minute>(a) <
             std::get<distributions::d_delay_minute>(b);
}

void read_distribution_probabilities(
    bool const is_first_distribution_element,
    std::vector<distributions_csv> const& distributions_entries,
    unsigned int const entry_index, std::vector<probability>& probabilities) {

  /* In the distributions-file, minutes with a probability equal 0 are ignored.
   * In order to initialize a probability_distribution object, we need
   * a vector containing the probabilities of all minutes without gaps. */
  if (!is_first_distribution_element) {
    unsigned int const num_missing_enteries =
        (std::get<distributions::d_delay_minute>(
             distributions_entries[entry_index]) -
         std::get<distributions::d_delay_minute>(
             distributions_entries[entry_index - 1])) -
        1;
    for (unsigned int i = 0; i < num_missing_enteries; i++)
      probabilities.push_back(0.0);
  }

  probabilities.push_back(std::get<distributions::d_delay_probability>(
      distributions_entries[entry_index]));
}

void load_distributions(
    std::string const filepath,
    std::vector<probability_distribution>& probability_distributions) {
  std::vector<distributions_csv> distributions_entries;
  parser::read_file<distributions_csv, ';'>(
      filepath.c_str(), distributions_entries, distributions_columns);
  std::sort(distributions_entries.begin(), distributions_entries.end(),
            distribution_is_smaller);

  struct distribution_info {
    distribution_info(unsigned int const distribution_id, int const first_min)
        : distribution_id_(distribution_id), first_min_(first_min) {}
    unsigned int const distribution_id_;
    int const first_min_;
    std::vector<probability> probabilities_;
  };
  std::vector<distribution_info> distribution_infos;

  for (unsigned int i = 0; i < distributions_entries.size(); i++) {
    bool const is_first_distribution_element =
        (i == 0 ||
         std::get<distributions::d_distribution_id>(distributions_entries[i]) !=
             std::get<distributions::d_distribution_id>(
                 distributions_entries[i - 1]));

    // add new distribution entry if necessary
    if (is_first_distribution_element) {
      distribution_infos.emplace_back(
          (unsigned int)std::get<distributions::d_distribution_id>(
              distributions_entries[i]),
          std::get<distributions::d_delay_minute>(
              distributions_entries[i])); /* first delay minute */
    }

    read_distribution_probabilities(
        is_first_distribution_element, distributions_entries, i,
        distribution_infos[distribution_infos.size() - 1].probabilities_);
  }

  probability_distributions.resize(distribution_infos.size());
  for (unsigned int i = 0; i < distribution_infos.size(); i++) {
    probability_distributions[i].init(distribution_infos[i].probabilities_,
                                      distribution_infos[i].first_min_);
  }
}

bool is_smaller(mapping_csv a, mapping_csv b) {
  return std::get<mapping::m_distribution_id>(a) <
             std::get<mapping::m_distribution_id>(a) &&
         std::get<mapping::m_distribution_class>(a) <
             std::get<mapping::m_distribution_class>(b) &&
         std::get<mapping::m_from_travel_time>(a) <
             std::get<mapping::m_from_travel_time>(b) &&
         std::get<mapping::m_to_travel_time>(a) <
             std::get<mapping::m_to_travel_time>(b) &&
         std::get<mapping::m_from_delay>(a) <
             std::get<mapping::m_from_delay>(b) &&
         std::get<mapping::m_to_delay>(a) < std::get<mapping::m_to_delay>(b);
}
void load_distribution_mappings(std::string const filepath) {
  std::vector<mapping_csv> mapping_entries;
  parser::read_file<mapping_csv, ';'>(filepath.c_str(), mapping_entries,
                                      mapping_columns);
  for (auto m : mapping_entries)
    std::cout << std::get<mapping::m_distribution_id>(m) << ";"
              << std::get<mapping::m_distribution_class>(m) << ";"
              << std::get<mapping::m_from_travel_time>(m) << ";"
              << std::get<mapping::m_to_travel_time>(m) << ";"
              << std::get<mapping::m_from_delay>(m) << ";"
              << std::get<mapping::m_to_delay>(m) << std::endl;
}

void load_start_distributions(std::string const filepath) {}

}  // namespace detail
}  // namespace distributions_loader
}  // namespace reliability
}  // namespace motis
