#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/context.h"
#include "motis/reliability/distributions/s_t_distributions_container.h"
#include "motis/reliability/error.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/realtime/realtime_update.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/late_connections.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"
#include "motis/reliability/tools/flatbuffers/response_builder.h"

#include "../test/include/start_and_travel_test_distributions.h"

using namespace motis::logging;
using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

#define READ_DISTRIBUTINS "reliability.read_distributions"
#define DISTRIBUTIONS_FOLDERS "reliability.distributions_folders"
#define HOTELS_FILE "reliability.hotels"

namespace motis {
namespace reliability {

reliability::reliability()
    : read_distributions_(false),
      distributions_folders_(
          {"/data/db_distributions/train/", "/data/db_distributions/bus/"}),
      hotels_file_("modules/reliability/resources/hotels.csv") {}

po::options_description reliability::desc() {
  po::options_description desc("Reliability Module");
  // clang-format off
  desc.add_options()
      (READ_DISTRIBUTINS,
       po::value<bool>(&read_distributions_)->
       default_value(read_distributions_),
       "read start and travel time distributions");
  desc.add_options()
      (DISTRIBUTIONS_FOLDERS,
       po::value<std::vector<std::string>>(&distributions_folders_)->
       default_value(distributions_folders_)->multitoken(),
       "folders containing distributions");
  desc.add_options()
      (HOTELS_FILE,
       po::value<std::string>(&hotels_file_)->
       default_value(hotels_file_),
       "file containing hotels info");
  // clang-format on
  return desc;
}

void reliability::print(std::ostream& out) const {
  out << "  " << READ_DISTRIBUTINS << ": " << read_distributions_ << "\n  "
      << DISTRIBUTIONS_FOLDERS << ": " << distributions_folders_ << "\n  "
      << HOTELS_FILE << ": " << hotels_file_;
}

std::vector<s_t_distributions_container::parameters>
get_s_t_distributions_parameters(std::vector<std::string> const& paths) {
  std::vector<s_t_distributions_container::parameters> param;
  for (auto const& p : paths) {
    if (p.rfind("/train/") != std::string::npos) {
      param.push_back({p, 500, 120});  // TODO: read max travel time from graph
    } else if (p.rfind("/bus/") != std::string::npos) {
      param.push_back({p, 60, 45});
    } else if (p.rfind("/str/") != std::string::npos) {
      param.push_back({p, 20, 31});
    } else {
      LOG(logging::warn) << "Undefined distribution type";
    }
  }
  return param;
}

void reliability::init() {
  auto lock = synced_sched();
  schedule& sched = lock.sched();
  precomputed_distributions_ =
      std::unique_ptr<distributions_container::container>(
          new distributions_container::container);
  if (read_distributions_) {
    s_t_distributions_ = std::unique_ptr<start_and_travel_distributions>(
        new s_t_distributions_container(
            get_s_t_distributions_parameters(distributions_folders_)));
    LOG(info) << "Read start and travel distributions from files";
  } else {
    s_t_distributions_ = std::unique_ptr<start_and_travel_distributions>(
        new start_and_travel_test_distributions({0.8, 0.2}, {0.1, 0.8, 0.1},
                                                -1));
    LOG(info) << "Using generated start and travel distributions";
  }
  distributions_calculator::precomputation::perform_precomputation(
      sched, *s_t_distributions_, *precomputed_distributions_);
}

void reliability::on_msg(msg_ptr msg, sid session_id, callback cb) {
  auto content_type = msg->msg_->content_type();
  if (content_type == MsgContent_ReliableRoutingRequest) {
    auto req = msg->content<ReliableRoutingRequest const*>();
    return handle_routing_request(req, session_id, cb);
  } else if (content_type == MsgContent_RealtimeDelayInfoResponse) {
    auto update =
        msg->content<motis::realtime::RealtimeDelayInfoResponse const*>();
    return handle_realtime_update(update, cb);
  }
  return cb({}, error::not_implemented);
}

void reliability::handle_routing_request(ReliableRoutingRequest const* req,
                                         sid session_id, callback cb) {
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
      return search::late_connections::search(
          req, *this, session_id,
          std::bind(&reliability::handle_late_connection_result, this, p::_1,
                    p::_2, cb));
    }
    default: break;
  }
  return cb({}, error::not_implemented);
}

void reliability::handle_realtime_update(
    motis::realtime::RealtimeDelayInfoResponse const* res, callback cb) {
  LOG(info) << "reliability received " << res->delay_infos()->size()
            << " delay infos";

  auto lock = synced_sched();
  schedule& sched = lock.sched();
  realtime::update_precomputed_distributions(res, sched, *s_t_distributions_,
                                             *precomputed_distributions_);
  return cb({}, error::ok);
}

void reliability::handle_routing_response(msg_ptr msg,
                                          boost::system::error_code e,
                                          callback cb) {
  if (e) {
    return cb(nullptr, e);
  }
  auto lock = synced_sched();
  schedule& schedule = lock.sched();
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
  auto res = flatbuffers::response_builder::to_reliable_routing_response(cgs);
  std::ofstream os("CG.json");
  os << res->to_json() << std::endl;
  return cb(res, error::ok);
}

void reliability::handle_late_connection_result(motis::module::msg_ptr msg,
                                                boost::system::error_code,
                                                motis::module::callback cb) {
  auto res = msg->content<routing::RoutingResponse const*>();
  auto const journeys = message_to_journeys(res);
  auto lock = synced_sched();
  schedule& sched = lock.sched();
  std::cout << "\n\n\nJourneys found:\n\n";
  for (auto const& j : journeys) {
    print_journey(j, sched.schedule_begin_, std::cout);
    std::cout << std::endl;
  }
  return cb(msg, error::ok);
}

void reliability::send_message(msg_ptr msg, sid session, callback cb) {
  return dispatch(msg, session, cb);
}

}  // namespace reliability
}  // namespace motis
