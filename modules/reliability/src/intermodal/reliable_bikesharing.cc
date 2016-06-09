#include "motis/reliability/intermodal/reliable_bikesharing.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "motis/module/context/motis_call.h"
#include "motis/module/module.h"

#include "motis/protocol/ReliableRoutingRequest_generated.h"
#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/reliability/error.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {
namespace detail {

std::vector<std::pair<time_t, time_t>> compress_intervals(
    std::vector<std::pair<time_t, time_t>> orig_intervals) {
  std::sort(orig_intervals.begin(), orig_intervals.end());
  std::vector<std::pair<time_t, time_t>> compressed;
  for (auto const& i : orig_intervals) {
    if (!compressed.empty() && (i.first - compressed.back().second) <= 60) {
      compressed.back().second = i.second;
    } else {
      compressed.push_back(i);
    }
  }
  return compressed;
}

std::vector<bikesharing_info> const to_bikesharing_infos(
    ::flatbuffers::Vector<::flatbuffers::Offset<
        ::motis::bikesharing::BikesharingEdge>> const& edges,
    availability_aggregator const& aggregator, unsigned const max_duration) {
  std::vector<bikesharing_info> infos;
  for (auto edge : edges) {
    auto const bike_duration =
        static_cast<unsigned>(edge->bike_duration() / 60);
    auto const walk_duration =
        static_cast<unsigned>(edge->walk_duration() / 60);
    if (bike_duration + walk_duration > max_duration) {
      continue;
    }
    std::vector<std::pair<time_t, time_t>> availability_intervals;
    for (auto rating : *edge->availability()) {
      if (aggregator.is_reliable(rating->value())) {
        availability_intervals.emplace_back(
            static_cast<time_t>(rating->begin()),
            static_cast<time_t>(rating->end()));
      }
    }
    auto const intervals = compress_intervals(availability_intervals);
    if (!intervals.empty()) {
      infos.push_back({bike_duration, walk_duration, edge->station_id()->str(),
                       bikesharing_info::terminal{edge->from()->pos()->lat(),
                                                  edge->from()->pos()->lng()},
                       bikesharing_info::terminal{edge->to()->pos()->lat(),
                                                  edge->to()->pos()->lng()},
                       intervals});
    }
  }
  return infos;
}

std::vector<bikesharing_info> pareto_filter(
    std::vector<bikesharing_info> const& all) {
  auto is_optimal = [&all](bikesharing_info const& i) {
    auto dominates = [](bikesharing_info const& a, bikesharing_info const& b) {
      auto ratio = [](bikesharing_info const& i) {
        return static_cast<float>(i.walk_duration_) /
               static_cast<float>(i.bike_duration_);
      };
      auto const a_ratio = ratio(a), b_ratio = ratio(b);
      if (a.station_eva_ == b.station_eva_ && a_ratio <= b_ratio &&
          a.duration() <= b.duration() &&
          (a_ratio < b_ratio || a.duration() < b.duration())) {
        return true;
      }
      return false;
    };
    return std::find_if(all.begin(), all.end(), [&](auto const& a) {
             return dominates(a, i);
           }) == all.end();
  };

  std::vector<bikesharing_info> optimal_set;
  for (auto const& i : all) {
    if (is_optimal(i)) {
      optimal_set.push_back(i);
    }
  }

  return optimal_set;
}

}  // namespace detail

std::vector<bikesharing_info> retrieve_bikesharing_infos(
    bool for_departure, ReliableRoutingRequest const& req,
    bool const reliable_only, unsigned const max_duration,
    bool const pareto_filtering_for_bikesharing) {
  auto res = motis_call(to_bikesharing_request(
                            req, for_departure,
                            motis::bikesharing::AvailabilityAggregator_Average))
                 ->val();
  motis::reliability::intermodal::bikesharing::average_aggregator aggregator(
      reliable_only ? 4 : 0);
  using ::motis::bikesharing::BikesharingResponse;
  auto const all = detail::to_bikesharing_infos(
      *motis_content(BikesharingResponse, res)->edges(), aggregator,
      max_duration);
  return pareto_filtering_for_bikesharing ? detail::pareto_filter(all) : all;
}

module::msg_ptr to_bikesharing_request(
    ReliableRoutingRequest const& req, bool const for_departure,
    motis::bikesharing::AvailabilityAggregator const aggregator) {
  if ((for_departure && !req.dep_is_intermodal()) ||
      (!for_departure && !req.arr_is_intermodal())) {
    throw std::system_error(error::failure);
  }
  std::time_t begin, end;
  if (req.request()->start_type() == routing::Start_PretripStart) {
    auto start =
        reinterpret_cast<routing::PretripStart const*>(req.request()->start());
    begin = start->interval()->begin();
    end = start->interval()->end();
  } else if (req.request()->start_type() == routing::Start_OntripStationStart) {
    auto start = reinterpret_cast<routing::OntripStationStart const*>(
        req.request()->start());
    begin = start->departure_time();
    end = start->departure_time() + 60;
  } else {
    throw std::system_error(error::not_implemented);
  }

  auto const coord = for_departure ? req.dep_coord() : req.arr_coord();

  return to_bikesharing_request(for_departure, coord->lat(), coord->lng(),
                                begin, end, aggregator);
}

module::msg_ptr to_bikesharing_request(
    bool const is_departure_type, double const lat, double const lng,
    time_t const window_begin, time_t const window_end,
    motis::bikesharing::AvailabilityAggregator const aggregator) {
  module::message_creator fb;
  Position pos(lat, lng);
  Interval window(window_begin, window_end);
  fb.create_and_finish(
      MsgContent_BikesharingRequest,
      motis::bikesharing::CreateBikesharingRequest(
          fb, is_departure_type ? motis::bikesharing::Type_Departure
                                : motis::bikesharing::Type_Arrival,
          &pos, &window, aggregator)
          .Union(),
      "/bikesharing");
  return module::make_msg(fb);
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
