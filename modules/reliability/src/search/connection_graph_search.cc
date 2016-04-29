#include "motis/reliability/search/connection_graph_search.h"

#include <limits>
#include <memory>

#include "motis/module/context/motis_call.h"
#include "motis/module/context/motis_parallel_for.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/core/access/time_access.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/rating/connection_graph_rating.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"
#include "motis/reliability/search/connection_graph_search_tools.h"

namespace p = std::placeholders;

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace detail {

std::vector<journey> retrieve_base_journeys(
    ReliableRoutingRequest const* request) {
  auto routing_response =
      motis_call(flatbuffers::request_builder(request->request())
                     .build_routing_request())
          ->val();
  using routing::RoutingResponse;
  return message_to_journeys(motis_content(RoutingResponse, routing_response));
}

void init_connection_graph_from_base_journey(context& context,
                                             journey const& base_journey) {
  context.connection_graphs_.emplace_back();
  auto& cg_context = context.connection_graphs_.back();
  cg_context.index_ = context.connection_graphs_.size() - 1;

  connection_graph_builder::add_base_journey(*cg_context.cg_, base_journey);
  cg_context.stop_states_.resize(cg_context.cg_->stops_.size());
  rating::cg::rate_inserted_alternative(cg_context, 0,
                                        context.reliability_context_);
}

void add_alternative(journey const& j, std::shared_ptr<context> c,
                     context::conn_graph_context& cg_context,
                     unsigned int const stop_idx) {
  connection_graph_builder::add_alternative_journey(*cg_context.cg_, stop_idx,
                                                    j);
  cg_context.stop_states_.resize(cg_context.cg_->stops_.size());
  rating::cg::rate_inserted_alternative(cg_context, stop_idx,
                                        c->reliability_context_);
}

std::vector<unsigned int> init_active_stops(
    context::conn_graph_context& cg,
    connection_graph_optimizer const& optimizer) {
  std::vector<unsigned int> active;
  for (auto const& stop : cg.cg_->stops_) {
    if (stop.index_ != connection_graph::stop::Index_departure_stop &&
        stop.index_ != connection_graph::stop::Index_arrival_stop &&
        !optimizer.complete(stop, cg.stop_states_.at(stop.index_))) {
      active.push_back(stop.index_);
    }
  }
  return active;
}

struct request_type {
  unsigned int stop_id_;
  module::msg_ptr request_msg_;
  context::journey_cache_key cache_key_;
};

std::shared_ptr<request_type> create_alternative_request(
    connection_graph const& cg, unsigned int const stop_index,
    duration const min_dep_diff) {
  auto const req =
      tools::to_routing_request(cg, cg.stops_.at(stop_index), min_dep_diff);
  return std::make_shared<request_type>(
      request_type{stop_index, req.first, req.second});
}

journey retrieve_alternative(motis::module::msg_ptr const& request) {
  printf("REQ: %s", request->to_json().c_str());

  std::vector<journey> journeys;
  try {
    auto routing_response = motis_call(request)->val();
    using routing::RoutingResponse;
    journeys =
        message_to_journeys(motis_content(RoutingResponse, routing_response));
  } catch (...) {
    LOG(logging::warn) << "Failed to retrieve alternative";
  }

  /* note: this method ignores journeys that are
   * corrupt because the state machine in journey.cc
   * can not handle walks at the beginning of journeys
   * (such journeys are found in the on-trip search).
   * This filtering is not necessary as soon as the state
   * machine in journey.cc works correctly. */
  auto const filtered = tools::remove_invalid_journeys(journeys);
  if (filtered.size() != journeys.size() || filtered.empty()) {
    return journey();
  }
  return tools::select_alternative(filtered);
}

struct alternative_futures {
  struct future_return {
    std::shared_ptr<request_type> request_;
    journey journey_;
    bool is_cached_;
  };

  std::vector<ctx::future_ptr<module::ctx_data, future_return>>
      new_alternative_futures_;

  void spawn_request(std::shared_ptr<request_type> req,
                     std::shared_ptr<context> c) {
    printf("\nREQ stop=%u eva=%s time=%s", req->stop_id_,
           req->cache_key_.from_eva_.c_str(),
           format_time(unix_to_motistime(
                           c->reliability_context_.schedule_.schedule_begin_,
                           req->cache_key_.ontrip_time_))
               .c_str());
    new_alternative_futures_.emplace_back(module::spawn_job(
        req, [c](std::shared_ptr<request_type> req) -> future_return {
          auto const cache_it = c->journey_cache_.find(req->cache_key_);
          bool const is_cached = cache_it != c->journey_cache_.end();
          return future_return{
              req, is_cached ? cache_it->second
                             : retrieve_alternative(req->request_msg_),
              is_cached};
        }));
  };
};

void build_cg(context::conn_graph_context& cg, std::shared_ptr<context> c) {
  auto active_stops = init_active_stops(cg, *c->optimizer_);
  if (active_stops.empty()) {
    return;
  }

  // TODO mutex for cache ?

  alternative_futures futures;
  do {
    for (auto const stop_id : active_stops) {
      futures.spawn_request(
          create_alternative_request(*cg.cg_, stop_id,
                                     c->optimizer_->min_departure_diff_),
          c);
    }
    active_stops.clear();

    auto const alternative = futures.new_alternative_futures_.front()->val();
    if (!alternative.journey_.stops_.empty()) {
      print_journey(alternative.journey_,
                    c->reliability_context_.schedule_.schedule_begin_,
                    std::cout);
      if (!alternative.is_cached_) {
        c->journey_cache_[alternative.request_->cache_key_] =
            alternative.journey_;
      }
      add_alternative(alternative.journey_, c, cg,
                      alternative.request_->stop_id_);
      if (!c->optimizer_->complete(
              cg.cg_->stops_.at(alternative.request_->stop_id_),
              cg.stop_states_.at(alternative.request_->stop_id_))) {
        active_stops.push_back(alternative.request_->stop_id_);
      }
    } else {
      printf("\nStop %u set to completed since no alternative found",
             alternative.request_->stop_id_);
    }
    futures.new_alternative_futures_.erase(
        futures.new_alternative_futures_.begin());
  } while (!(futures.new_alternative_futures_.empty() && active_stops.empty()));
}

}  // namespace detail

std::vector<std::shared_ptr<connection_graph>> search_cgs(
    ReliableRoutingRequest const* request,
    motis::reliability::context const& rel_context,
    std::shared_ptr<connection_graph_optimizer const> optimizer) {
  auto c = std::make_shared<detail::context>(rel_context, optimizer);

  for (auto const& j : detail::retrieve_base_journeys(request)) {
    detail::init_connection_graph_from_base_journey(*c, j);
    print_journey(j, rel_context.schedule_.schedule_begin_, std::cout);
  }

  std::cout << "\nNum base journeys: " << c->connection_graphs_.size()
            << std::endl;

  using namespace motis::module;
  motis_parallel_for(c->connection_graphs_,
                     std::bind(&detail::build_cg, std::placeholders::_1, c));

  std::vector<std::shared_ptr<connection_graph>> cgs;
  for (auto const& cg_c : c->connection_graphs_) {
    cgs.push_back(cg_c.cg_);
  }
  return cgs;
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
