#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "motis/module/sid.h"
#include "motis/module/message.h"
#include "motis/protocol/BikesharingRequest_generated.h"

#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
namespace reliability {
struct reliability;
namespace intermodal {
namespace bikesharing {

struct availability_aggregator {
  virtual ~availability_aggregator() {}
  virtual motis::bikesharing::AvailabilityAggregator get_aggregator() const = 0;
  virtual bool is_reliable(double const&) const = 0;
};

struct average_aggregator : availability_aggregator {
  average_aggregator(double const threshold) : threshold_(threshold) {}
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
  std::string station_eva_;
  unsigned int duration_;
  /* right-open intervals */
  std::vector<std::pair<time_t, time_t>> availability_intervals_;
  std::pair<std::string, std::string> bikesharing_stations_;
};

/* bikesharing at departure (first) and at arrival (second) */
typedef std::function<void(std::pair<std::vector<bikesharing_info>,
                                     std::vector<bikesharing_info>> const)>
    callback;

/* retrieves reliable bikesharing infos */
void retrieve_bikesharing_infos(module::msg_ptr request,
                                std::shared_ptr<availability_aggregator>,
                                motis::reliability::reliability&, module::sid,
                                callback);

module::msg_ptr to_bikesharing_request(
    double const departure_lat, double const departure_lng,
    double const arrival_lat, double const arrival_lng,
    std::time_t window_begin, std::time_t window_end,
    motis::bikesharing::AvailabilityAggregator);

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
