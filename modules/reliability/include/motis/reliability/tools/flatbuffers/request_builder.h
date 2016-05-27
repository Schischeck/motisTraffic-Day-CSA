#pragma once

#include <ctime>
#include <string>
#include <tuple>

#include "motis/module/module.h"

namespace motis {
namespace routing {
struct RoutingRequest;  // NOLINT
}  // namespace routing
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
namespace intermodal {
struct individual_modes_container;
namespace bikesharing {
struct bikesharing_info;
}  // namespace bikesharing
}  // namespace intermodal
namespace flatbuffers {

struct request_builder {
  explicit request_builder(
      routing::SearchType search_type = routing::SearchType_DefaultForward);

  /* for bcg-base journeys and late connections */
  explicit request_builder(routing::RoutingRequest const&);

  /* to build routing-requests */
  explicit request_builder(ReliableRoutingRequest const&);

  request_builder& add_pretrip_start(std::string const& name,
                                     std::string const& id,
                                     std::time_t const interval_begin,
                                     std::time_t const interval_end);
  request_builder& add_ontrip_station_start(std::string const& name,
                                            std::string const& id,
                                            std::time_t const ontrip_time);
  request_builder& add_destination(std::string const& name,
                                   std::string const& id);

  /* for reliable intermodal requests */
  request_builder& add_intermodal_start(double const& lat, double const& lng,
                                        std::time_t const interval_begin,
                                        std::time_t const interval_end);
  request_builder& add_intermodal_destination(double const& lat,
                                              double const& lng);

  request_builder& add_additional_edge(
      ::flatbuffers::Offset<routing::AdditionalEdgeWrapper> const&);

  request_builder& add_additional_edges(
      intermodal::individual_modes_container const&);

  ::flatbuffers::Offset<routing::RoutingRequest> create_routing_request();

  module::msg_ptr build_routing_request();

  module::msg_ptr build_reliable_search_request(int16_t const min_dep_diff,
                                                bool const bikesharing = false);

  module::msg_ptr build_rating_request(bool const bikesharing = false);

  module::msg_ptr build_late_connection_request();

  module::msg_ptr build_connection_tree_request(
      int16_t const num_alternatives_at_stop, int16_t const min_dep_diff);

  module::message_creator b_;
  routing::SearchType search_type_;
  std::pair<routing::Start, ::flatbuffers::Offset<void>> start_;
  ::flatbuffers::Offset<routing::InputStation> destination_station_;
  std::vector<::flatbuffers::Offset<routing::AdditionalEdgeWrapper>>
      additional_edges_;

  /* for reliable intermodal requests */
  bool dep_is_intermodal_, arr_is_intermodal_;
  struct coordinates {
    double lat_, lng_;
  } dep_, arr_;

private:
  /* not for intermodal requests */
  void init_start_from_routing_request(routing::RoutingRequest const&);

  module::msg_ptr build_reliable_request(
      ::flatbuffers::Offset<RequestOptionsWrapper> const&,
      bool const bikesharing = false);

  void create_pretrip_start(std::string const station_name,
                            std::string const station_id,
                            std::time_t const interval_begin,
                            std::time_t const interval_end);
};

}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
