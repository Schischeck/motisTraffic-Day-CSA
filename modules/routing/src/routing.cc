#include "motis/routing/routing.h"

#include <iostream>

#include "boost/program_options.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/module/api.h"

#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/routing/label.h"
#include "motis/routing/search.h"
#include "motis/routing/response_builder.h"

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

msg_ptr routing::on_msg(msg_ptr const& msg, sid session) {
  auto req = msg->content<RoutingRequest const*>();

  if (req->path()->Length() < 2) {
    return {};
  }

  arrival_part from, to;
  auto from_eva = req->path()->Get(0)->eva_nr();
  auto to_eva = req->path()->Get(req->path()->Length() - 1)->eva_nr();
  from.station = schedule_->eva_to_station[from_eva]->index;
  to.station = schedule_->eva_to_station[to_eva]->index;

  auto interv_begin = unix_to_motis_time(*schedule_, req->interval()->begin());
  auto interv_end = unix_to_motis_time(*schedule_, req->interval()->begin());

  search s(*schedule_, label_store_);
  auto journeys = s.get_connections({from}, {to}, interv_begin, interv_end);
  return journeys_to_message(journeys);
}

MOTIS_MODULE_DEF_MODULE(routing)

}  // namespace routing
}  // namespace motis
