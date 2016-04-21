#include "motis/routing/routing.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/util.h"
#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/module/error.h"
#include "motis/module/motis_call.h"
#include "motis/module/get_schedule.h"

#include "motis/routing/additional_edges.h"
#include "motis/routing/query_to_arrivals.h"
#include "motis/routing/search.h"
#include "motis/routing/error.h"

#define LABEL_MEMORY_NUM_BYTES "routing.label_store_size"

namespace p = std::placeholders;
namespace po = boost::program_options;
namespace fbs = flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::guesser;

namespace motis {
namespace routing {

std::string get_eva(StationPathElement const* el) {
  if (el->eva_nr()->Length() != 0) {
    return el->eva_nr()->str();
  }
  MessageCreator b;
  b.CreateAndFinish(MsgContent_StationGuesserRequest,
                    motis::guesser::CreateStationGuesserRequest(
                        b, 1, b.CreateString(el->name()->str()))
                        .Union(),
                    "/guesser");
  auto msg = motis_call(make_msg(b))->val();
  auto guesses = motis_content(StationGuesserResponse, msg)->guesses();
  if (guesses->size() == 0) {
    throw boost::system::system_error(error::no_guess_for_station);
  }
  return guesses->Get(0)->eva()->str();
}

std::string get_eva(CoordinatesPathElement const* el) {
  return el->is_source() == 1 ? "-1" : "-2";
}

std::vector<arrival> get_arrivals(
    fbs::Vector<fbs::Offset<LocationPathElementWrapper>> const* path) {
  std::vector<arrival> arrivals;
  for (auto const& el : *path) {
    std::string station_id;
    if (el->element_type() == LocationPathElement_StationPathElement) {
      station_id =
          get_eva(reinterpret_cast<StationPathElement const*>(el->element()));
    } else if (el->element_type() ==
               LocationPathElement_CoordinatesPathElement) {
      station_id = get_eva(
          reinterpret_cast<CoordinatesPathElement const*>(el->element()));
    } else {
      throw boost::system::system_error(
          error::unsupported_location_path_element);
    }

    auto it = get_schedule().eva_to_station.find(station_id);
    if (it == end(get_schedule().eva_to_station)) {
      throw boost::system::system_error(error::given_eva_not_available);
    }
    arrivals.push_back({arrival_part(it->second->index)});
  }
  return arrivals;
}

routing::routing()
    : label_bytes_(static_cast<uint64_t>(8) * 1024 * 1024 * 1024) {}

po::options_description routing::desc() {
  po::options_description desc("Routing Module");
  // clang-format off
  desc.add_options()
    (LABEL_MEMORY_NUM_BYTES,
     po::value<std::size_t>(&label_bytes_)->default_value(label_bytes_),
     "size of the label store in bytes");
  // clang-format on
  return desc;
}

void routing::print(std::ostream& out) const {
  out << "  " << LABEL_MEMORY_NUM_BYTES << ": " << label_bytes_;
}

void routing::init(motis::module::registry& reg) {
  label_store_ = make_unique<memory_manager>(label_bytes_);
  reg.register_op("/routing", std::bind(&routing::route, this, p::_1));
}

msg_ptr routing::route(msg_ptr const& msg) {
  auto req = motis_content(RoutingRequest, msg);
  if (req->path()->Length() < 2) {
    throw boost::system::system_error(error::path_length_too_short);
  }

  auto& sched = get_schedule();
  if (req->interval()->begin() < static_cast<unsigned>(sched.schedule_begin_) ||
      req->interval()->end() >= static_cast<unsigned>(sched.schedule_end_)) {
    throw boost::system::system_error(error::journey_date_not_in_schedule);
  }

  auto const arrivals = get_arrivals(req->path());
  auto const arrivals_pair = query_to_arrivals::create_arrivals(
      arrivals[0], arrivals[1], req->additional_edges(),
      req->type() == Type_OnTrip || req->type() == Type_LateConnection, sched);

  auto const i_begin =
      unix_to_motistime(sched.schedule_begin_, req->interval()->begin());
  auto const i_end =
      unix_to_motistime(sched.schedule_begin_, req->interval()->end());

  search s(sched, *label_store_);
  auto result = s.get_connections(
      arrivals_pair.first, arrivals_pair.second, i_begin, i_end,
      req->type() == Type_OnTrip || req->type() == Type_LateConnection,
      create_additional_edges(
          req->additional_edges(),
          req->type() == Type_OnTrip || req->type() == Type_LateConnection,
          sched));

  LOG(info) << sched.stations[arrivals[0][0].station]->name << " to "
            << sched.stations[arrivals[1][0].station]->name << " "
            << "(" << format_time(i_begin) << ", " << format_time(i_end)
            << ") -> " << result.journeys.size() << " connections found";

  // TODO connection checker annotation
  return journeys_to_message(result.journeys, result.stats.pareto_dijkstra);
}

}  // namespace routing
}  // namespace motis
