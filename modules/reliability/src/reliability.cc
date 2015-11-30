#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/context.h"
#include "motis/reliability/db_distributions.h"
#include "motis/reliability/error.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/late_connections.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"
#include "motis/reliability/tools/flatbuffers/response_builder.h"

#include "../test/include/start_and_travel_test_distributions.h"

using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

#define HOTELS_FILE "reliability.hotels"

namespace motis {
namespace reliability {

reliability::reliability()
    : hotels_file_("modules/reliability/resources/hotels.csv") {}

po::options_description reliability::desc() {
  po::options_description desc("Reliability Module");
  // clang-format off
  desc.add_options()
      (HOTELS_FILE,
       po::value<std::string>(&hotels_file_)->default_value(hotels_file_),
       "file containing hotels info");
  // clang-format on
  return desc;
}

void reliability::print(std::ostream& out) const {
  out << "  " << HOTELS_FILE << ": " << hotels_file_;
}

void reliability::init() {
  auto const lock = synced_sched();
  schedule const& schedule = lock.sched();
  precomputed_distributions_ = std::unique_ptr<
      distributions_container::precomputed_distributions_container>(
      new distributions_container::precomputed_distributions_container(
          schedule.node_count));
#ifdef USE_DB_DISTRIBUTINS
  s_t_distributions_ = std::unique_ptr<
      start_and_travel_distributions>(new db_distributions(
      "/home/keyhani/Workspace/git/motis/DBDists/DBData/20130805/Original/td/",
      500,
      120));  // TODO: read max travel time from graph
  std::cout << "\ndb-distributions" << std::endl;
#else
  s_t_distributions_ = std::unique_ptr<start_and_travel_distributions>(
      new start_and_travel_test_distributions({0.8, 0.2}, {0.1, 0.8, 0.1}, -1));
#endif
  distributions_calculator::precomputation::perform_precomputation(
      schedule, *s_t_distributions_, *precomputed_distributions_);
}

void reliability::on_msg(msg_ptr msg, sid session_id, callback cb) {
  auto req = msg->content<ReliableRoutingRequest const*>();
  switch (req->request_type()->request_options_type()) {
    case RequestOptions_RatingReq: {
      return dispatch(
          flatbuffers::request_builder::to_flatbuffers_message(req->request()),
          session_id, std::bind(&reliability::handle_routing_response, this,
                                p::_1, p::_2, cb));
    }
    case RequestOptions_ReliableSearchReq: {
      auto req_info =
          (ReliableSearchReq const*)req->request_type()->request_options();
      return search::connection_graph_search::search_cgs(
          req, *this, session_id,
          std::make_shared<
              search::connection_graph_search::reliable_cg_optimizer>(
              req_info->min_departure_diff()),
          std::bind(&reliability::handle_connection_graph_result, this, p::_1,
                    cb));
    }
    case RequestOptions_ConnectionTreeReq: {
      auto req_info =
          (ConnectionTreeReq const*)req->request_type()->request_options();
      return search::connection_graph_search::search_cgs(
          req, *this, session_id,
          std::make_shared<search::connection_graph_search::simple_optimizer>(
              req_info->num_alternatives_at_each_stop(),
              req_info->min_departure_diff()),
          std::bind(&reliability::handle_connection_graph_result, this, p::_1,
                    cb));
    }
    case RequestOptions_LateConnectionReq: {
      return search::late_connections::search(req, *this, session_id, cb);
    }
    default: break;
  }
  return cb({}, error::not_implemented);
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
  auto const journeys = message_to_journeys(res);
  try {
    for (auto const& j : journeys) {
      rating::rate(
          ratings[rating_index], j,
          context(schedule, *precomputed_distributions_, *s_t_distributions_));
      rating::simple_rating::rate(simple_ratings[rating_index], j, schedule,
                                  *s_t_distributions_);
      ++rating_index;
    }
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return cb(nullptr, error::failure);
  }

  cb(flatbuffers::response_builder::to_reliability_rating_response(
         res, ratings, simple_ratings, true /* short output */),
     error::ok);
}

void reliability::handle_connection_graph_result(
    std::vector<std::shared_ptr<search::connection_graph>> const cgs,
    callback cb) {
  return cb(flatbuffers::response_builder::to_reliable_routing_response(cgs),
            error::ok);
}

void reliability::send_message(msg_ptr msg, sid session, callback cb) {
  return dispatch(msg, session, cb);
}

}  // namespace reliability
}  // namespace motis
