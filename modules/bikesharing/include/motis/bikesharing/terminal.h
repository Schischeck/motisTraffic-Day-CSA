#pragma once

#include <ctime>
#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "motis/protocol/BikesharingCommon_generated.h"

namespace motis {
namespace bikesharing {

struct terminal {
  std::string uid_;
  double lat_, lng_;
  std::string name_;
};

struct terminal_snapshot : public terminal {
  int available_bikes_;
};

struct availability {
  double average_;
  double median_;
  double minimum_;
  double q90_;
  double percent_reliable_;
};

struct close_location {
  std::string id_;
  int duration_;
};

constexpr size_t kHoursPerDay = 24;
constexpr size_t kDaysPerWeek = 7;
constexpr size_t kBucketCount = kHoursPerDay * kDaysPerWeek;

template <typename T>
using hourly = std::array<T, kBucketCount>;
using hourly_availabilities = hourly<availability>;

size_t timestamp_to_bucket(std::time_t timestamp);

struct snapshot_merger {
  using hourly_buckets = hourly<std::vector<int>>;

  void add_snapshot(std::time_t, std::vector<terminal_snapshot> const&);

  std::pair<std::vector<terminal>, std::vector<hourly_availabilities>> merged();

  size_t snapshot_count_ = 0;
  std::map<std::string, terminal> terminals_;
  std::map<std::string, hourly_buckets> distributions_;
};

struct Availability;
double get_availability(Availability const*, AvailabilityAggregator);

}  // namespace bikesharing
}  // namespace motis
