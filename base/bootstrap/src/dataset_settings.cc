#include "motis/bootstrap/dataset_settings.h"

#include <ostream>

#define DATASET "dataset.path"
#define WRITE_SERIALIZED "dataset.write_serialized"
#define APPLY_RULES "dataset.apply_rules"
#define UNIQUE_CHECK "dataset.unique_check"
#define SCHEDULE_BEGIN "dataset.begin"
#define NUM_DAYS "dataset.num_days"

namespace motis {
namespace bootstrap {

namespace po = boost::program_options;

dataset_settings::dataset_settings(std::string default_dataset,
                                   bool write_serialized, bool apply_rules,
                                   bool unique_check,
                                   std::string schedule_begin, int num_days)
    : loader_options(default_dataset, write_serialized, apply_rules,
                     unique_check, schedule_begin, num_days) {}

po::options_description dataset_settings::desc() {
  po::options_description desc("Dataset Settings");
  // clang-format off
  desc.add_options()
      (DATASET,
       po::value<std::string>(&dataset)->default_value(dataset),
       "MOTIS Dataset root")
      (WRITE_SERIALIZED,
       po::value<bool>(&write_serialized)->default_value(write_serialized),
       "Ignore serialized dataset")
      (UNIQUE_CHECK,
       po::value<bool>(&unique_check)->default_value(unique_check),
       "Check for duplicates and don't integrate them")
      (APPLY_RULES,
       po::value<bool>(&apply_rules)->default_value(apply_rules),
       "Apply special rules (through-services, merge-split-services)")
      (SCHEDULE_BEGIN,
       po::value<std::string>(&schedule_begin)->default_value(schedule_begin),
       "schedule interval begin (TODAY or YYYYMMDD)")
      (NUM_DAYS,
       po::value<int>(&num_days)->default_value(num_days),
       "number of days");
  // clang-format on
  return desc;
}

void dataset_settings::print(std::ostream& out) const {
  out << "  " << DATASET << ": " << dataset << "\n"
      << "  " << WRITE_SERIALIZED << ": " << write_serialized << "\n"
      << "  " << UNIQUE_CHECK << ": " << unique_check << "\n"
      << "  " << APPLY_RULES << ": " << apply_rules << "\n"
      << "  " << SCHEDULE_BEGIN << ": " << schedule_begin << "\n"
      << "  " << NUM_DAYS << ": " << num_days;
}

}  // namespace bootstrap
}  // namespace motis
