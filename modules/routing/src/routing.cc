#include "motis/routing/routing.h"

#include <iostream>

#include "boost/program_options.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/protocol/StationGuesserRequest_generated.h"

#include "motis/routing/label.h"
#include "motis/routing/search.h"
#include "motis/routing/response_builder.h"

using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace routing {

routing::routing() : label_store_(MAX_LABELS) {}

po::options_description routing::desc() {
  po::options_description desc("Routing Module");
  return desc;
}

void routing::print(std::ostream& out) const {}

time unix_to_motis_time(schedule const& s, uint64_t unix_timestamp) {
  auto first_date = s.date_mgr.first_date();
  boost::posix_time::ptime schedule_begin(boost::gregorian::date(
      first_date.year, first_date.month, first_date.day));
  boost::posix_time::ptime query_time =
      boost::posix_time::from_time_t(unix_timestamp);
  return ((query_time - schedule_begin).total_milliseconds() / 1000) / 60;
}

arrival routing::read_path_element(StationPathElement const* el) {
  auto eva = el->eva_nr();

  if (eva == 0) {
    FlatBufferBuilder b;
    b.Finish(
        CreateMessage(b, MsgContent_StationGuesserRequest,
                      motis::guesser::CreateStationGuesserRequest(
                          b, 1, b.CreateString(el->name()->str())).Union()));
    auto res = (*dispatch_)(make_msg(b), 0);

    if (!res) {
      std::cout << "no response\n";
      return {};
    }

    if (res->content_type() != MsgContent_StationGuesserResponse) {
      std::cout << "wrong content type " << res->content_type() << "\n";
      return {};
    }

    auto guess = res->content<motis::guesser::StationGuesserResponse const*>();
    if (guess->guesses()->Length() == 0) {
      std::cout << "no guesses for " << el->name()->c_str() << "\n";
      return {};
    }

    eva = guess->guesses()->Get(0)->eva();
  }

  auto station_it = schedule_->eva_to_station.find(eva);
  if (station_it == end(schedule_->eva_to_station)) {
    return {};
  }

  return {arrival_part(station_it->second->index)};
}

msg_ptr routing::on_msg(msg_ptr const& msg, sid session) {
  auto req = msg->content<RoutingRequest const*>();

  if (req->path()->Length() < 2) {
    std::cout << "request does not contain at least 2 path elements\n";
    return {};
  }

  auto from = read_path_element(req->path()->Get(0));
  auto to = read_path_element(req->path()->Get(req->path()->Length() - 1));

  auto interv_begin = unix_to_motis_time(*schedule_, req->interval()->begin());
  auto interv_end = unix_to_motis_time(*schedule_, req->interval()->end());

  search s(*schedule_, label_store_);
  auto journeys = s.get_connections({from}, {to}, interv_begin, interv_end);

  LOG(info) << schedule_->stations[from[0].station]->name << " to "
            << schedule_->stations[to[0].station]->name << " "
            << "(" << format_time(interv_begin) << ", "
            << format_time(interv_end) << ") -> " << journeys.size()
            << " connections found";

  return journeys_to_message(journeys);
}

MOTIS_MODULE_DEF_MODULE(routing)

}  // namespace routing
}  // namespace motis
