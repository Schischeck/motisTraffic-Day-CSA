#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/core/common/journey_builder.h"

#include "motis/reliability/db_distributions.h"
#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/error.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "../test/include/start_and_travel_test_distributions.h"

using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace reliability {

po::options_description reliability::desc() {
  po::options_description desc("Reliability Module");
  return desc;
}

void reliability::print(std::ostream&) const {}

void reliability::init() {
  auto const lock = synced_sched<RO>();
  schedule const& schedule = lock.sched();
  precomputed_distributions_ = std::unique_ptr<
      distributions_container::precomputed_distributions_container>(
      new distributions_container::precomputed_distributions_container(
          schedule.node_count));
  /*s_t_distributions_ = std::unique_ptr<start_and_travel_distributions>(
      new db_distributions("", 120,
                           120));  // TODO: read max travel time from graph*/
  s_t_distributions_ = std::unique_ptr<start_and_travel_distributions>(
      new start_and_travel_test_distributions({0.8, 0.2}, {0.1, 0.8, 0.1}, -1));
  distributions_calculator::precomputation::perform_precomputation(
      schedule, *s_t_distributions_, *precomputed_distributions_);
}

void reliability::on_msg(msg_ptr msg, sid session_id, callback cb) {
  if (msg->content_type() == MsgContent_ReliableRoutingRequest) {
    auto req = msg->content<ReliableRoutingRequest const*>();
    find_and_rate_connections(req->request(), session_id, cb);
    return dispatch(flatbuffers_tools::to_flatbuffers_message(req->request()),
                    session_id, std::bind(&reliability::handle_routing_response,
                                          this, p::_1, p::_2, cb));
  } else {
    return cb({}, error::not_implemented);
  }
}

void reliability::find_and_rate_connections(
    routing::RoutingRequest const* request, sid session_id, callback cb) {
  return dispatch(
      flatbuffers_tools::to_flatbuffers_message(request), session_id,
      std::bind(&reliability::handle_routing_response, this, p::_1, p::_2, cb));
}

void reliability::handle_routing_response(motis::module::msg_ptr msg,
                                          boost::system::error_code e,
                                          callback cb) {
  if (e) {
    return cb(nullptr, e);
  }

  std::cout << "\n\n------------" << std::endl;
  auto const lock = synced_sched<RO>();
  schedule const& schedule = lock.sched();
  auto res = msg->content<routing::RoutingResponse const*>();
  std::vector<rating::connection_rating> ratings(res->connections()->size());
  unsigned int rating_index = 0;
  for (auto it = res->connections()->begin(); it != res->connections()->end();
       ++it, ++rating_index) {
    rating::rate(ratings[rating_index], *it, schedule,
                 *precomputed_distributions_, *s_t_distributions_);
    {
      std::cout << "\nconnection: " << std::flush;
      for (auto it2 = it->transports()->begin(); it2 != it->transports()->end();
           ++it2) {
        if (it2->move_type() == routing::Move_Transport) {
          auto transport = (routing::Transport const*)it2->move();
          std::cout << transport->train_nr() << " " << std::flush;
          auto const& r = std::find_if(
              ratings[rating_index].public_transport_ratings.begin(),
              ratings[rating_index].public_transport_ratings.end(),
              [&](rating::rating_element const& r) {
                return transport->range()->to() == r.arrival_stop_idx();
              });
          std::cout << r->arrival_distribution_ << std::endl;
        }
      }
      std::cout << ratings[rating_index]
                       .public_transport_ratings.back()
                       .arrival_distribution_ << std::endl;
    }
  }

  return cb(
      msg /*flatbuffers_tools::to_reliable_routing_response(msg, ratings)*/,
      error::ok);
}

}  // namespace reliability
}  // namespace motis
