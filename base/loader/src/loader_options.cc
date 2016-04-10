#include "motis/loader/loader_options.h"

#include "boost/date_time/local_time/local_time.hpp"

#include "motis/core/common/date_util.h"

namespace motis {
namespace loader {

loader_options::loader_options(std::string default_dataset,
                               bool write_serialized, bool apply_rules,
                               bool adjust_footpaths, bool unique_check,
                               std::string schedule_begin, int num_days)
    : dataset_(std::move(default_dataset)),
      write_serialized_(write_serialized),
      unique_check_(unique_check),
      apply_rules_(apply_rules),
      adjust_footpaths_(adjust_footpaths),
      schedule_begin_(std::move(schedule_begin)),
      num_days_(num_days) {}

std::pair<std::time_t, std::time_t> loader_options::interval() const {
  std::pair<std::time_t, std::time_t> interval;

  if (schedule_begin_ == "TODAY") {
    auto now = boost::posix_time::second_clock::universal_time().date();
    interval.first = to_unix_time(now.year(), now.month(), now.day());
  } else {
    interval.first = to_unix_time(std::stoi(schedule_begin_.substr(0, 4)),
                                  std::stoi(schedule_begin_.substr(4, 2)),
                                  std::stoi(schedule_begin_.substr(6, 2)));
  }

  interval.second = interval.first + num_days_ * 24 * 3600;

  return interval;
}

}  // namespace loader
}  // namespace motis
