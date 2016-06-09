#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "motis/module/message.h"
#include "motis/protocol/BikesharingRequest_generated.h"

#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
namespace routing {
struct RoutingRequest;  // NOLINT
}  // namespace routing
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
namespace intermodal {
namespace bikesharing {

struct availability_aggregator {
  virtual ~availability_aggregator() = default;
  virtual motis::bikesharing::AvailabilityAggregator get_aggregator() const = 0;
  virtual bool is_reliable(double const&) const = 0;
};

struct average_aggregator : availability_aggregator {
  explicit average_aggregator(double const threshold) : threshold_(threshold) {}
  motis::bikesharing::AvailabilityAggregator get_aggregator() const override {
    return motis::bikesharing::AvailabilityAggregator::
        AvailabilityAggregator_Average;
  }
  bool is_reliable(double const& val) const override {
    return greater_equal(val, threshold_);
  }

private:
  double const threshold_;
};

struct bikesharing_info {
  unsigned int duration() const { return bike_duration_ + walk_duration_; }
  unsigned int bike_duration_, walk_duration_;
  std::string station_eva_;
  struct terminal {
    double lat_, lng_;
  } from_, to_;
  /* right-open intervals */
  std::vector<std::pair<time_t, time_t>> availability_intervals_;
};

/* retrieves reliable bikesharing infos for departure or arrival */
std::vector<bikesharing_info> retrieve_bikesharing_infos(
    bool const for_departure, ReliableRoutingRequest const&,
    bool const reliable_only, unsigned const max_duration,
    bool const pareto_filtering_for_bikesharing);

module::msg_ptr to_bikesharing_request(
    bool const is_departure_type, double const lat, double const lng,
    time_t const window_begin, time_t const window_end,
    motis::bikesharing::AvailabilityAggregator const aggregator);

module::msg_ptr to_bikesharing_request(
    ReliableRoutingRequest const&, bool const for_departure,
    motis::bikesharing::AvailabilityAggregator const);

namespace detail {
std::vector<bikesharing_info> const to_bikesharing_infos(
    ::flatbuffers::Vector<
        ::flatbuffers::Offset<::motis::bikesharing::BikesharingEdge>> const&,
    availability_aggregator const&, unsigned const max_duration);
std::vector<std::pair<time_t, time_t>> compress_intervals(
    std::vector<std::pair<time_t, time_t>> orig_intervals);
std::vector<bikesharing_info> pareto_filter(
    std::vector<bikesharing_info> const&);
}  // namespace detail

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
