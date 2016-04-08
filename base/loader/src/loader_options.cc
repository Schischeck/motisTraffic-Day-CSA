#include "motis/loader/loader_options.h"

#include "boost/lexical_cast.hpp"
#include "boost/date_time/local_time/local_time.hpp"

#include "motis/core/common/date_time_util.h"

namespace motis {
namespace loader {

loader_options::loader_options(std::string default_dataset,
                               bool write_serialized, bool apply_rules,
                               bool adjust_footpaths, bool unique_check,
                               std::string schedule_begin, int num_days)
    : dataset(std::move(default_dataset)),
      write_serialized(write_serialized),
      unique_check(unique_check),
      apply_rules(apply_rules),
      adjust_footpaths(adjust_footpaths),
      schedule_begin(schedule_begin),
      num_days(num_days) {}

std::pair<std::time_t, std::time_t> loader_options::interval() const {
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

}  // namespace loader
}  // namespace motis
