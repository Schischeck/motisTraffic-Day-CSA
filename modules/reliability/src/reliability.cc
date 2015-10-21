#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

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
    return dispatch(flatbuffers_tools::to_flatbuffers_message(req->request()),
                    session_id, std::bind(&reliability::handle_routing_response,
                                          this, p::_1, p::_2, cb));
  } else {
    return cb({}, error::not_implemented);
  }
}

void reliability::handle_routing_response(motis::module::msg_ptr msg,
                                          boost::system::error_code e,
                                          callback cb) {
  if (e) {
    return cb(nullptr, e);
  }
  auto const lock = synced_sched<RO>();
  schedule const& schedule = lock.sched();
  auto res = msg->content<routing::RoutingResponse const*>();
  std::vector<rating::connection_rating> ratings(res->connections()->size());
  unsigned int rating_index = 0;
  for (auto it = res->connections()->begin(); it != res->connections()->end();
       ++it, ++rating_index) {
    bool success =
        rating::rate(ratings[rating_index], *it, schedule,
                     *precomputed_distributions_, *s_t_distributions_);
    if (!success) {
      std::cout << "\nError(reliability) could not rate the connections"
                << std::endl;
      return cb(nullptr, error::failure);
    }
  }

  return cb(flatbuffers_tools::to_reliable_routing_response(
                res, schedule.categories, ratings, true /* short output */),
            error::ok);
}

}  // namespace reliability
}  // namespace motis
