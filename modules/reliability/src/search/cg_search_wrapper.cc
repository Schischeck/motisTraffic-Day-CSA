#include "motis/reliability/search/connection_graph_search.h"

#include <memory>
#include <vector>

#include "motis/module/message.h"
#include "motis/protocol/ReliableRoutingRequest_generated.h"

#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"
#include "motis/reliability/tools/flatbuffers/response_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace detail {

std::shared_ptr<connection_graph_optimizer> get_optimizer(
    RequestOptionsWrapper const& options) {
  if (options.request_options_type() == RequestOptions_ReliableSearchReq) {
    auto req_info =
        reinterpret_cast<ReliableSearchReq const*>(options.request_options());
    return std::make_shared<reliable_cg_optimizer>(
        req_info->min_departure_diff());
  } else if (options.request_options_type() ==
             RequestOptions_ConnectionTreeReq) {
    auto req_info =
        reinterpret_cast<ConnectionTreeReq const*>(options.request_options());
    return std::make_shared<simple_optimizer>(
        req_info->num_alternatives_at_each_stop(),
        req_info->min_departure_diff());
  }
  throw std::system_error(error::not_implemented);
}

void update_mumo_info(
    std::vector<std::shared_ptr<search::connection_graph>>& cgs,
    intermodal::individual_modes_container const& container) {
  for (auto& cg : cgs) {
    for (auto& j : cg->journeys_) {
      intermodal::update_mumo_info(j, container);
    }
  }
}

void update_address_info(
    ReliableRoutingRequest const& req,
    std::vector<std::shared_ptr<search::connection_graph>>& cgs) {
  for (auto& cg : cgs) {
    if (req.dep_is_intermodal()) {
      cg->journeys_.front().stops_.front().name_ =
          departure_station_name(*req.request());
    }
    if (req.arr_is_intermodal()) {
      std::set<uint16_t> arriving_journeys;
      for (auto& s : cg->stops_) {
        for (auto& a : s.alternative_infos_) {
          if (a.next_stop_index_ ==
              connection_graph::stop::INDEX_ARRIVAL_STOP) {
            arriving_journeys.insert(a.journey_index_);
          }
        }
      }
      for (auto idx : arriving_journeys) {
        cg->journeys_.at(idx).stops_.back().name_ =
            req.request()->destination()->name()->str();
      }
    }
  }
}

}  // namespace detail

module::msg_ptr search_cgs(ReliableRoutingRequest const& req, reliability& rel,
                           unsigned const max_bikesharing_duration,
                           bool const pareto_filtering_for_bikesharing) {
  auto lock = rel.synced_sched();
  intermodal::individual_modes_container container(
      req, max_bikesharing_duration, pareto_filtering_for_bikesharing);
  auto cgs = search_cgs(req, ::motis::reliability::context(
                                 lock.sched(), *rel.precomputed_distributions_,
                                 *rel.s_t_distributions_),
                        detail::get_optimizer(*req.request_type()), container);
  detail::update_mumo_info(cgs, container);
  detail::update_address_info(req, cgs);
  return response_builder::to_reliable_routing_response(cgs);
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
