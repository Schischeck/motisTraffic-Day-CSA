#include "motis/loader/waiting_time_rules_parser.h"

namespace motis {
namespace loader {

waiting_time_rules parse_waiting_time_rules(
    std::vector<std::string> const& category_names) {
  waiting_time_rules rules;

  rules.default_group = 5;

  // clang-format off
  std::vector<std::vector<std::string>> categories{
  { "EC", "ICE", "IC", "Tha", "CIS", "RHT", "RHI" },
  { "EN", "NZ", "D", "CNL", "TLG", "DNZ" },
  { "IRE", "RE", "RB" },
  { "S", "s" },
  { "DPE", "DPN", "R", "IRX", "X", "E", "SCH", "BSV", "RT", "FB", "LX", "REX" }
  };
  // clang-format on

  // clang-format off
  std::vector<int> waiting_times{
  3, 0, 0, 0, 0,
  10, 10, 0, 0, 5,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  5, 5, 0, 5, 5
  };
  // clang-format on

  // category/group numbers are 1-based.
  // initializes all values to false.
  int number_of_groups = categories.size();
  rules._waits_for_other_trains.resize(number_of_groups + 1);
  rules._other_trains_wait_for.resize(number_of_groups + 1);
  rules._waiting_time_matrix = flat_matrix<duration>(number_of_groups + 1);

  for (int i = 0; i < number_of_groups; i++) {
    int group = i + 1;  // groups are one-based
    for (auto const& category_name : categories[i]) {
      rules._category_map[category_name] = group;
    }
  }

  for (int i = 0; i < number_of_groups * number_of_groups; i++) {
    int connecting_cat = i / number_of_groups + 1;
    int feeder_cat = i % number_of_groups + 1;
    int waiting_time = waiting_times[i];

    rules._waiting_time_matrix[connecting_cat][feeder_cat] = waiting_time;

    if (waiting_time > 0) {
      rules._waits_for_other_trains[connecting_cat] = true;
      rules._other_trains_wait_for[feeder_cat] = true;
    }
  }

  rules._family_to_wtr_category.resize(category_names.size());
  for (size_t i = 0; i < category_names.size(); i++) {
    rules._family_to_wtr_category[i] =
        rules.waiting_time_category(category_names[i]);
  }

  return rules;
}

}  // loader
}  // motis
