#pragma once

#include <map>
#include <memory>
#include <mutex>

#include "motis/module/module.h"

#include "motis/protocol/ReliableRoutingRequest_generated.h"
#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/probability_distribution.h"

#include "motis/reliability/intermodal/individual_modes_container.h"

namespace motis {
struct journey;
namespace reliability {
namespace search {
struct connection_graph;
namespace connection_graph_search {
struct connection_graph_optimizer;
namespace detail {

struct context {
  context(motis::reliability::context rel_context,
          std::shared_ptr<connection_graph_optimizer const> optimizer,
          ReliableRoutingRequest const& req)
      : reliability_context_(std::move(rel_context)) /* NOLINT */,
        optimizer_(std::move(optimizer)),
        individual_modes_container_(req) {
    destination_.is_intermodal_ = req.arr_is_intermodal();
    if (destination_.is_intermodal_) {
      destination_.coordinates_.lat_ = req.arr_coord()->lat();
      destination_.coordinates_.lng_ = req.arr_coord()->lng();
      destination_.station_.id_ = "";
      destination_.station_.name_ = "";
    } else {
      auto const& routing_req =
          *reinterpret_cast<routing::RoutingRequest const*>(req.request());
      destination_.station_.id_ = routing_req.destination()->id()->str();
      destination_.station_.name_ = routing_req.destination()->name()->str();
      destination_.coordinates_.lat_ = 0.0;
      destination_.coordinates_.lng_ = 0.0;
    }
  }

  motis::reliability::context reliability_context_;
  std::shared_ptr<connection_graph_optimizer const> optimizer_;

  struct conn_graph_context {
    conn_graph_context()
        : index_(0), cg_(std::make_shared<connection_graph>()) {}
    unsigned int index_;
    std::shared_ptr<connection_graph> cg_;

    struct stop_state {
      probability_distribution uncovered_arrival_distribution_;
    };

    /* positions analogous to connection_graph::stops_ */
    std::vector<stop_state> stop_states_;
  };
  std::vector<conn_graph_context> connection_graphs_;

  struct journey_cache_key {
    journey_cache_key() = default;
    journey_cache_key(std::string from_eva, time_t const& ontrip_time)
        : from_eva_(std::move(from_eva)), ontrip_time_(ontrip_time) {}
    bool operator<(journey_cache_key const& right) const {
      return from_eva_ < right.from_eva_ || (from_eva_ == right.from_eva_ &&
                                             ontrip_time_ < right.ontrip_time_);
    }
    std::string from_eva_;
    time_t ontrip_time_;
  };
  std::pair<std::mutex, std::map<journey_cache_key, journey>> journey_cache_;

  /* destination is either a gps-position or a station */
  struct {
    bool is_intermodal_;
    struct {
      double lat_, lng_;
    } coordinates_;
    struct station {
      std::string id_, name_;
    } station_;
  } destination_;

  intermodal::individual_modes_container individual_modes_container_;
};

}  // namespace detail
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
