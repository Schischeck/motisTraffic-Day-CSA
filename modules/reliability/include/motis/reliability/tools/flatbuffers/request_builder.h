#pragma once

#include <ctime>
#include <string>
#include <tuple>

#include "motis/module/module.h"

#include "motis/protocol/RoutingRequest_generated.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {
struct bikesharing_infos;
}  // namespace bikesharing
}  // namespace intermodal
namespace flatbuffers {

struct request_builder {
  request_builder(
      routing::SearchType search_type = routing::SearchType_DefaultForward);
  request_builder(routing::RoutingRequest const*);

  request_builder& add_pretrip_start(std::string const& name,
                                     std::string const& id,
                                     std::time_t const interval_begin,
                                     std::time_t const interval_end);
  request_builder& add_ontrip_station_start(std::string const& name,
                                            std::string const& id,
                                            std::time_t const ontrip_time);
  request_builder& add_destination(std::string const& name,
                                   std::string const& eva);

  /* for reliable intermodal requests */
  request_builder& add_dep_coordinates(double const& lat, double const& lng);
  request_builder& add_arr_coordinates(double const& lat, double const& lng);

  request_builder& add_additional_edge(
      ::flatbuffers::Offset<routing::AdditionalEdgeWrapper> const&);

  request_builder& add_additional_edges(
      motis::reliability::intermodal::bikesharing::bikesharing_infos const&);

  ::flatbuffers::Offset<routing::RoutingRequest> create_routing_request();

  module::msg_ptr build_routing_request();

  module::msg_ptr build_reliable_search_request(int16_t const min_dep_diff,
                                                bool const bikesharing = false);

  module::msg_ptr build_rating_request(bool const bikesharing = false);

  module::msg_ptr build_late_connection_cequest();

  module::msg_ptr build_connection_tree_request(
      int16_t const num_alternatives_at_stop, int16_t const min_dep_diff);

  module::message_creator b_;
  routing::SearchType search_type_;
  std::pair<routing::Start, ::flatbuffers::Offset<void>> start_;
  ::flatbuffers::Offset<routing::InputStation> destination_station_;
  std::vector<::flatbuffers::Offset<routing::AdditionalEdgeWrapper>>
      additional_edges_;

  /* for reliable intermodal requests */
  bool is_intermodal_;
  struct coordinates {
    double lat_, lng_;
  } dep_, arr_;

private:
  module::msg_ptr build_reliable_request(
      ::flatbuffers::Offset<RequestOptionsWrapper> const&,
      bool const bikesharing = false);
};

}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
