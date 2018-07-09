#include "motis/loader/wzr_loader.h"

namespace motis {
namespace loader {

waiting_time_rules load_waiting_time_rules(
    std::vector<std::unique_ptr<category>> const& category_ptrs) {
  waiting_time_rules rules;

  rules.default_group_ = 6;

  // clang-format off
  std::vector<std::vector<std::string>> categories{
  { "EC", "ICE", "IC", "Tha", "CIS", "RHT", "RHI" },
  { "EN", "NZ", "D", "CNL", "TLG", "DNZ" },
  { "IRE", "RE", "RB" },
  { "S", "s" },
  { "DPE", "DPN", "R", "IRX", "X", "E", "SCH", "BSV", "RT", "FB", "LX", "REX" },
  { } /* default group */
  };
  // clang-format on

  // clang-format off
  std::vector<int> waiting_times{
  3, 0, 0, 0, 0, 0,
  10, 10, 0, 0, 5, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  5, 5, 0, 5, 5, 0,
  0, 0, 0, 0, 0, 0 /* default group */
  };
  // clang-format on

  // category/group numbers are 1-based.
  // initializes all values to false.
  int number_of_groups = categories.size();
  rules.waits_for_other_trains_.resize(number_of_groups + 1);
  rules.other_trains_wait_for_.resize(number_of_groups + 1);
  rules.waiting_time_matrix_ = flat_matrix<uint32_t>(number_of_groups + 1);

  for (int i = 0; i < number_of_groups; i++) {
    int group = i + 1;  // groups are one-based
    for (auto const& category_name : categories[i]) {
      rules.category_map_[category_name] = group;
    }
  }

  for (int i = 0; i < number_of_groups * number_of_groups; i++) {
    int connecting_cat = i / number_of_groups + 1;
    int feeder_cat = i % number_of_groups + 1;
    int waiting_time = waiting_times[i];

    rules.waiting_time_matrix_[connecting_cat][feeder_cat] = waiting_time;

    if (waiting_time > 0) {
      rules.waits_for_other_trains_[connecting_cat] = true;
      rules.other_trains_wait_for_[feeder_cat] = true;
    }
  }

  rules.family_to_wtr_category_.resize(category_ptrs.size());
  for (size_t i = 0; i < category_ptrs.size(); i++) {
    rules.family_to_wtr_category_[i] =
        rules.waiting_time_category(category_ptrs[i]->name_);
  }

  return rules;
}

}  // namespace loader
}  // namespace motis
