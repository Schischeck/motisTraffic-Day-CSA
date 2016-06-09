#include "motis/reliability/reliability.h"

#include <iostream>
#include <memory>
#include <string>

#include "boost/algorithm/string.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/util.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/context.h"
#include "motis/reliability/distributions/s_t_distributions_container.h"
#include "motis/reliability/error.h"
#include "motis/reliability/rating/reliability_rating.h"
#include "motis/reliability/realtime/realtime_update.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/late_connections.h"

#include "../test/include/start_and_travel_test_distributions.h"

using namespace motis::logging;
using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

#define READ_DISTRIBUTINS "reliability.read_distributions"
#define DISTRIBUTIONS_FOLDERS "reliability.distributions_folders"
#define HOTELS_FILE "reliability.hotels"
#define MAX_BIKESHARING_DURATION "reliability.max_bikesharing_duration"
#define PARETO_FILTERING "reliability.pareto_filtering_for_bikesharing"

namespace motis {
namespace reliability {

reliability::reliability()
    : read_distributions_(false),
      distributions_folders_(
          {"/data/db_distributions/train/", "/data/db_distributions/bus/"}),
      hotels_file_("modules/reliability/resources/hotels.csv"),
      max_bikesharing_duration_(45),
      pareto_filtering_for_bikesharing_(true) {}

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
  desc.add_options()
      (MAX_BIKESHARING_DURATION,
       po::value<unsigned>(&max_bikesharing_duration_)->
       default_value(max_bikesharing_duration_),
       "maximum allowed duration for bikesharing");
  desc.add_options()
        (PARETO_FILTERING,
         po::value<bool>(&pareto_filtering_for_bikesharing_)->
         default_value(pareto_filtering_for_bikesharing_),
         "activate pareto-filtering for bikesharings");
  // clang-format on
  return desc;
}

void reliability::print(std::ostream& out) const {
  out << "  " << READ_DISTRIBUTINS << ": " << read_distributions_ << "\n  "
      << DISTRIBUTIONS_FOLDERS << ": " << distributions_folders_ << "\n  "
      << HOTELS_FILE << ": " << hotels_file_ << "\n  "
      << MAX_BIKESHARING_DURATION << ": " << max_bikesharing_duration_ << "\n  "
      << PARETO_FILTERING << ": " << pareto_filtering_for_bikesharing_;
}

std::vector<s_t_distributions_container::parameters>
get_s_t_distributions_parameters(std::vector<std::string> const& paths) {
  std::vector<s_t_distributions_container::parameters> param;
  for (auto const& p : paths) {
    if (p.rfind("/train/") != std::string::npos) {
      // TODO(Mohammad Keyhani): read max travel time from graph
      param.push_back({p, 500, 120});
    } else if (p.rfind("/bus/") != std::string::npos) {
      param.push_back({p, 60, 45});
    } else if (p.rfind("/str/") != std::string::npos) {
      param.push_back({p, 20, 31});
    } else {
      LOG(logging::warn) << "undefined distribution type";
    }
  }
  return param;
}

void reliability::init(motis::module::registry& reg) {
  reg.register_op("/reliability/route",
                  std::bind(&reliability::routing_request, this, p::_1));
  reg.register_op("/reliability/update",
                  std::bind(&reliability::realtime_update, this, p::_1));

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

  precomputed_distributions_ =
      std::make_unique<distributions_container::container>();

  auto lock = synced_sched();
  distributions_calculator::precomputation::perform_precomputation(
      lock.sched(), *s_t_distributions_, *precomputed_distributions_);
}

msg_ptr reliability::routing_request(msg_ptr const& msg) {
  auto const& req = *motis_content(ReliableRoutingRequest, msg);
  switch (req.request_type()->request_options_type()) {
    case RequestOptions_RatingReq: {
      return rating::rating(req, *this, max_bikesharing_duration_,
                            pareto_filtering_for_bikesharing_);
    }
    case RequestOptions_ReliableSearchReq:
    case RequestOptions_ConnectionTreeReq: {
      return search::connection_graph_search::search_cgs(
          req, *this, max_bikesharing_duration_,
          pareto_filtering_for_bikesharing_);
    }
    case RequestOptions_LateConnectionReq: {
      return search::late_connections::search(req, *this, hotels_file_);
    }
    default: break;
  }
  throw std::system_error(error::not_implemented);
}

msg_ptr reliability::realtime_update(msg_ptr const&) {
  /* not implemented TODO(Mohammad Keyhani) */
  // auto lock = synced_sched();
  // auto& sched = lock.sched();
  // realtime::update_precomputed_distributions(res, sched, *s_t_distributions_,
  //                                           *precomputed_distributions_);
  throw std::system_error(error::not_implemented);
}

}  // namespace reliability
}  // namespace motis
