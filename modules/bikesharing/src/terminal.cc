#include "motis/bikesharing/terminal.h"

#include <numeric>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/local_timezone_defs.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"

#include "motis/core/common/logging.h"

using namespace motis::logging;

using ptime = boost::posix_time::ptime;
using date_t = ptime::date_type;
using dur_t = ptime::time_duration_type;

using dst_traits = boost::date_time::eu_dst_trait<date_t>;
using engine = boost::date_time::dst_calc_engine<date_t, dur_t, dst_traits>;
using adjustor = boost::date_time::local_adjustor<ptime, 1, engine>;

namespace motis {
namespace bikesharing {

size_t timestamp_to_bucket(std::time_t timestamp) {
  auto utc_time = boost::posix_time::from_time_t(timestamp);
  auto local_time = adjustor::utc_to_local(utc_time);

  auto weekday = local_time.date().day_of_week();
  auto hour = local_time.time_of_day().hours();

  return weekday * kHoursPerDay + hour;
}

void snapshot_merger::add_snapshot(
    std::time_t t, std::vector<terminal_snapshot> const& snapshots) {
  ++snapshot_count_;
  auto const& bucket = timestamp_to_bucket(t);
  for (auto const& s : snapshots) {
    terminals_[s.uid] = s;
    distributions_[s.uid][bucket].push_back(s.available_bikes);
  }
}

availability compute_availability(std::vector<int>& dist) {
  auto size = dist.size();
  availability a{0.0, 0, 0, 0, 0.0};
  if (size == 0) {
    return a;
  }

  auto sum = std::accumulate(begin(dist), end(dist), 0);
  a.average = static_cast<double>(sum) / size;

  std::sort(begin(dist), end(dist));
  a.median = dist[(size - 1) * 0.5];
  a.minimum = dist[0];
  a.q90 = dist[(size - 1) * 0.1];

  auto lb = std::lower_bound(begin(dist), end(dist), 5);
  a.percent_reliable = std::distance(lb, end(dist)) / size;

  return a;
}

std::pair<std::vector<terminal>, std::vector<hourly_availabilities>>
snapshot_merger::merged() {
  LOG(info) << "merging " << snapshot_count_ << " snapshots of "
            << terminals_.size() << " terminals";

  std::vector<terminal> t;
  std::vector<hourly_availabilities> a;
  for (auto& terminal : terminals_) {
    auto& id = terminal.first;

    hourly_availabilities availabilities;
    for (size_t i = 0; i < kBucketCount; ++i) {
      availabilities[i] = compute_availability(distributions_.at(id)[i]);
    }

    t.push_back(terminal.second);
    a.push_back(availabilities);
  }

  return {t, a};
}

}  // namespace bikesharing
}  // namespace motis
