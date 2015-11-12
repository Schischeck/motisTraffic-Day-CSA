#include "motis/webservice/dataset_settings.h"

#include <ostream>

#include "boost/lexical_cast.hpp"
#include "boost/date_time/local_time/local_time.hpp"

#include "motis/core/common/date_util.h"

#define DATASET "dataset.path"
#define USE_SERIALIZED "dataset.use_serialized"
#define SCHEDULE_BEGIN "dataset.begin"
#define NUM_DAYS "dataset.num_days"

namespace motis {
namespace webservice {

namespace po = boost::program_options;

dataset_settings::dataset_settings(std::string default_dataset,
                                   bool use_serialized,
                                   std::string schedule_begin, int num_days)
    : dataset(std::move(default_dataset)),
      use_serialized(use_serialized),
      schedule_begin(schedule_begin),
      num_days(num_days) {}

po::options_description dataset_settings::desc() {
  po::options_description desc("Dataset Settings");
  // clang-format off
  desc.add_options()
      (DATASET,
       po::value<std::string>(&dataset)->default_value(dataset),
       "MOTIS Dataset root")
      (USE_SERIALIZED,
       po::value<bool>(&use_serialized)->default_value(use_serialized),
       "Ignore serialized dataset")
      (SCHEDULE_BEGIN,
       po::value<std::string>(&schedule_begin)->default_value(schedule_begin),
       "schedule interval begin (TODAY or YYYYMMDD)")
      (NUM_DAYS,
       po::value<int>(&num_days)->default_value(num_days),
       "number of days");
  // clang-format on
  return desc;
}

std::pair<std::time_t, std::time_t> dataset_settings::interval() const {
  std::pair<std::time_t, std::time_t> interval;

  if (schedule_begin == "TODAY") {
    auto now = boost::posix_time::second_clock::universal_time().date();
    interval.first = to_unix_time(now.year(), now.month(), now.day());
  } else {
    interval.first =
        to_unix_time(boost::lexical_cast<int>(schedule_begin.substr(0, 4)),
                     boost::lexical_cast<int>(schedule_begin.substr(4, 2)),
                     boost::lexical_cast<int>(schedule_begin.substr(6, 2)));
  }

  interval.second = interval.first + num_days * 24 * 3600;

  return interval;
}

void dataset_settings::print(std::ostream& out) const {
  out << "  " << DATASET << ": " << dataset << "\n"
      << "  " << USE_SERIALIZED << ": " << use_serialized << "\n"
      << "  " << SCHEDULE_BEGIN << ": " << schedule_begin << "\n"
      << "  " << NUM_DAYS << ": " << num_days;
}

}  // namespace webservice
}  // namespace motis
