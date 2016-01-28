#pragma once

#include <array>
#include <ctime>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace motis {
namespace bikesharing {

struct terminal {
  std::string uid;
  double lat, lng;
  std::string name;
};

struct terminal_snapshot : public terminal {
  int available_bikes;
};

struct availability {
  double average;
  int median;
  int minimum;
  int q90;
  double percent_reliable;
};

struct close_location {
  std::string id;
  int duration;
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

  void add_snapshot(std::time_t timestamp,
                    std::vector<terminal_snapshot> const& snapshot);

  std::pair<std::vector<terminal>, std::vector<hourly_availabilities>> merged();

  size_t snapshot_count_ = 0;
  std::map<std::string, terminal> terminals_;
  std::map<std::string, hourly_buckets> distributions_;
};

}  // namespace bikesharing
}  // namespace motis
