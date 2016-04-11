#include "motis/bootstrap/dataset_settings.h"

#include <ostream>

#define DATASET "dataset.path"
#define WRITE_SERIALIZED "dataset.write_serialized"
#define APPLY_RULES "dataset.apply_rules"
#define ADJUST_FOOTPATHS "dataset.adjust_footpaths"
#define UNIQUE_CHECK "dataset.unique_check"
#define SCHEDULE_BEGIN "dataset.begin"
#define NUM_DAYS "dataset.num_days"

namespace motis {
namespace bootstrap {

namespace po = boost::program_options;

dataset_settings::dataset_settings(std::string default_dataset,
                                   bool write_serialized, bool apply_rules,
                                   bool adjust_footpaths, bool unique_check,
                                   std::string schedule_begin, int num_days)
    : loader_options(default_dataset, write_serialized, apply_rules,
                     adjust_footpaths, unique_check, schedule_begin, num_days) {
}

po::options_description dataset_settings::desc() {
  po::options_description desc("Dataset Settings");
  // clang-format off
  desc.add_options()
      (DATASET,
       po::value<std::string>(&dataset_)->default_value(dataset_),
       "MOTIS Dataset root")
      (WRITE_SERIALIZED,
       po::value<bool>(&write_serialized_)->default_value(write_serialized_),
       "Ignore serialized dataset")
      (UNIQUE_CHECK,
       po::value<bool>(&unique_check_)->default_value(unique_check_),
       "Check for duplicates and don't integrate them")
      (APPLY_RULES,
       po::value<bool>(&apply_rules_)->default_value(apply_rules_),
       "Apply special rules (through-services, merge-split-services)")
      (ADJUST_FOOTPATHS,
       po::value<bool>(&adjust_footpaths_)->default_value(adjust_footpaths_),
       "Remove footpaths if they do not fit an assumed average speed")
      (SCHEDULE_BEGIN,
       po::value<std::string>(&schedule_begin_)->default_value(schedule_begin_),
       "schedule interval begin (TODAY or YYYYMMDD)")
      (NUM_DAYS,
       po::value<int>(&num_days_)->default_value(num_days_),
       "number of days");
  // clang-format on
  return desc;
}

void dataset_settings::print(std::ostream& out) const {
  out << "  " << DATASET << ": " << dataset_ << "\n"
      << "  " << WRITE_SERIALIZED << ": " << write_serialized_ << "\n"
      << "  " << UNIQUE_CHECK << ": " << unique_check_ << "\n"
      << "  " << APPLY_RULES << ": " << apply_rules_ << "\n"
      << "  " << SCHEDULE_BEGIN << ": " << schedule_begin_ << "\n"
      << "  " << NUM_DAYS << ": " << num_days_;
}

}  // namespace bootstrap
}  // namespace motis
