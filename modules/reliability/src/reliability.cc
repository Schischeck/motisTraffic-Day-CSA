#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/db_distributions.h"
#include "motis/reliability/error.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/simple_connection_graph_optimizer.h"
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
  auto const lock = synced_sched();
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
  auto req = msg->content<ReliableRoutingRequest const*>();
  if (req->request_type() == RequestType_Rating) {
    return dispatch(flatbuffers_tools::to_flatbuffers_message(req->request()),
                    session_id, std::bind(&reliability::handle_routing_response,
                                          this, p::_1, p::_2, cb));
  }
  if (req->request_type() == RequestType_ReliableSearch) {
    return search::connection_graph_search::search_cgs(
        req, *this, session_id,
        search::connection_graph_search::simple_optimizer::complete,
        std::bind(&reliability::handle_connection_graph_result, this, p::_1,
                  cb));
  } else {
    return cb({}, error::not_implemented);
  }
}

void reliability::handle_routing_response(msg_ptr msg,
                                          boost::system::error_code e,
                                          callback cb) {
  if (e) {
    return cb(nullptr, e);
  }
  auto const lock = synced_sched();
  schedule const& schedule = lock.sched();
  auto res = msg->content<routing::RoutingResponse const*>();
  std::vector<rating::connection_rating> ratings(res->connections()->size());
  std::vector<rating::simple_rating::simple_connection_rating> simple_ratings(
      res->connections()->size());
  unsigned int rating_index = 0;
  for (auto it = res->connections()->begin(); it != res->connections()->end();
       ++it, ++rating_index) {
    bool success =
        rating::rate(ratings[rating_index], *it, schedule,
                     *precomputed_distributions_, *s_t_distributions_);
    success &= rating::simple_rating::rate(simple_ratings[rating_index], *it,
                                           schedule, *s_t_distributions_);
    if (!success) {
      std::cout << "\nError(reliability) could not rate the connections"
                << std::endl;
      return cb(nullptr, error::failure);
    }
  }

  return cb(flatbuffers_tools::to_reliability_rating_response(
                res, schedule.categories, ratings, simple_ratings,
                true /* short output */),
            error::ok);
}

void reliability::handle_connection_graph_result(
    std::vector<std::shared_ptr<search::connection_graph>> const cgs,
    callback cb) {
  return cb(flatbuffers_tools::to_reliable_routing_response(cgs), error::ok);
}

void reliability::send_message(msg_ptr msg, sid session, callback cb) {
  return dispatch(msg, session, cb);
}

}  // namespace reliability
}  // namespace motis
