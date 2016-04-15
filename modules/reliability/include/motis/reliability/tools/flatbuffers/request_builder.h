#pragma once

#include <ctime>
#include <string>
#include <tuple>

#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/module/module.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {
struct bikesharing_infos;
}
}
namespace flatbuffers {
namespace request_builder {

struct request_builder {
  request_builder(
      routing::Type const type = routing::Type::Type_PreTrip,
      routing::Direction const dir = routing::Direction::Direction_Forward);

  request_builder(routing::RoutingRequest const*);

  request_builder& add_station(std::string const& name, std::string const& eva);

  request_builder& add_coordinates(double const& lat, double const& lon,
                                   bool const is_source);

  request_builder& set_interval(std::time_t const begin, std::time_t const end);

  request_builder& set_interval(std::tuple<int, int, int> const ddmmyyyy,
                                motis::time const begin, motis::time const end);

  request_builder& add_additional_edge(
      ::flatbuffers::Offset<routing::AdditionalEdgeWrapper> const&);

  request_builder& add_additional_edges(
      motis::reliability::intermodal::bikesharing::bikesharing_infos const&);

  ::flatbuffers::Offset<routing::RoutingRequest> create_routing_request();

  module::msg_ptr build_routing_request();

  module::msg_ptr build_reliable_search_request(short const min_dep_diff,
                                                bool const bikesharing = false);

  module::msg_ptr build_rating_request(bool const bikesharing = false);

  module::msg_ptr build_late_connection_cequest();

  module::msg_ptr build_connection_tree_request(
      short const num_alternatives_at_stop, short const min_dep_diff);

  module::MessageCreator b_;
  routing::Type type_;
  routing::Direction direction_;
  std::vector<::flatbuffers::Offset<routing::LocationPathElementWrapper>> path_;
  std::time_t interval_begin_, interval_end_;
  std::vector<::flatbuffers::Offset<routing::AdditionalEdgeWrapper>>
      additional_edges_;

private:
  module::msg_ptr build_reliable_request(
      ::flatbuffers::Offset<RequestOptionsWrapper> const&,
      bool const bikesharing = false);
};

}  // namespace request_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
