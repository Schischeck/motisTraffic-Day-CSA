#include "motis/routing/routing.h"

#include <iostream>

#include "boost/program_options.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/core/common/util.h"
#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/module/error.h"

#include "motis/protocol/StationGuesserRequest_generated.h"

#include "motis/routing/label.h"
#include "motis/routing/search.h"
#include "motis/routing/error.h"

#define MAX_LABEL_COUNT "routing.max_label_count"

namespace p = std::placeholders;
namespace po = boost::program_options;
using boost::system::error_code;
using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;

namespace motis {
namespace routing {

routing::routing() : max_label_count_(MAX_LABELS_WITH_MARGIN) {}

po::options_description routing::desc() {
  po::options_description desc("Routing Module");
  // clang-format off
  desc.add_options()
    (MAX_LABEL_COUNT,
     po::value<int>(&max_label_count_)->default_value(max_label_count_),
     "number of labels to preallocate (crashes if not enough!)");
  // clang-format on
  return desc;
}

void routing::print(std::ostream&) const {}

void routing::read_path_element(StationPathElement const* el,
                                routing::path_el_cb cb) {
  auto eva = el->eva_nr();
  if (eva->size() == 0) {
    // Eva number not set: Try to guess entered station name.
    MessageCreator b;
    b.CreateAndFinish(MsgContent_StationGuesserRequest,
                      motis::guesser::CreateStationGuesserRequest(
                          b, 1, b.CreateString(el->name()->str()))
                          .Union());
    return dispatch(make_msg(b), 0, std::bind(&routing::handle_station_guess,
                                              this, p::_1, p::_2, cb));
  } else {
    // Eva number set: Try to get station using the eva_to_station map.
    auto lock = synced_sched<RO>();
    auto station_it = lock.sched().eva_to_station.find(eva->str());
    if (station_it == end(lock.sched().eva_to_station)) {
      return cb({}, error::given_eva_not_available);
    } else {
      return cb({arrival_part(station_it->second->index)}, error::ok);
    }
  }
}

void routing::init() {
  printf("allocating %d labels\n", max_label_count_);
  label_store_ = make_unique<memory_manager<label>>(max_label_count_);
}

void routing::handle_station_guess(msg_ptr res, error_code e,
                                   routing::path_el_cb cb) {
  if (e) {
    return cb({}, e);
  } else {
    auto guess = res->content<motis::guesser::StationGuesserResponse const*>();
    if (guess->guesses()->Length() == 0) {
      return cb({}, error::no_guess_for_station);
    } else {
      auto lock = synced_sched<RO>();
      auto eva = guess->guesses()->Get(0)->eva()->str();
      auto station_it = lock.sched().eva_to_station.find(eva);
      return cb({arrival_part(station_it->second->index)}, error::ok);
    }
  }
}

void routing::on_msg(msg_ptr msg, sid, callback cb) {
  auto req = msg->content<RoutingRequest const*>();

  if (req->path()->Length() < 2) {
    return cb({}, error::path_length_too_short);
  }

  auto path = std::make_shared<std::vector<arrival>>(req->path()->Length());
  auto path_complete = [path] {
    return std::all_of(begin(*path), end(*path),
                       [](arrival const& a) { return !a.empty(); });
  };
  auto path_cb = [=](int index, arrival arr, error_code e) {
    auto req = msg->content<RoutingRequest const*>();

    if (e) {
      return cb({}, e);
    }

    (*path)[index] = arr;

    if (!path_complete()) {
      return;
    }

    auto lock = synced_sched<schedule_access::RO>();
    auto const& sched = lock.sched();

    if (req->interval()->begin() <
            static_cast<unsigned>(sched.schedule_begin_) ||
        req->interval()->end() >= static_cast<unsigned>(sched.schedule_end_)) {
      return cb({}, error::journey_date_not_in_schedule);
    }

    auto i_begin =
        unix_to_motistime(sched.schedule_begin_, req->interval()->begin());
    auto i_end =
        unix_to_motistime(sched.schedule_begin_, req->interval()->end());

    search s(lock.sched(), *label_store_);
    auto journeys = s.get_connections(path->at(0), path->at(1), i_begin, i_end,
                                      req->type() != Type_PreTrip);

    LOG(info) << lock.sched().stations[path->at(0)[0].station]->name << " to "
              << lock.sched().stations[path->at(1)[0].station]->name << " "
              << "(" << format_time(i_begin) << ", " << format_time(i_end)
              << ") -> " << journeys.size() << " connections found";

    auto resp = journeys_to_message(journeys);
    return dispatch(resp, 0, [resp, cb](msg_ptr annotated, error_code e) {
      if (e == motis::module::error::no_module_capable_of_handling) {
        return cb(resp, error::ok);  // connectionchecker not available
      } else if (e) {
        return cb({}, e);
      } else {
        return cb(annotated, error::ok);
      }
    });
  };

  read_path_element(req->path()->Get(0), std::bind(path_cb, 0, p::_1, p::_2));
  read_path_element(req->path()->Get(req->path()->Length() - 1),
                    std::bind(path_cb, 1, p::_1, p::_2));

  return;
}

}  // namespace routing
}  // namespace motis
