#pragma once

#include <array>
#include <ctime>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace motis {
namespace bikesharing {

using terminal_id_t = int;

struct terminal {
  terminal_id_t uid;
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

  std::vector<std::pair<terminal, hourly_availabilities>> merged();

  size_t snapshot_count_ = 0;
  std::map<terminal_id_t, terminal> terminals_;
  std::map<terminal_id_t, hourly_buckets> distributions_;
};

}  // namespace bikesharing
}  // namespace motis
