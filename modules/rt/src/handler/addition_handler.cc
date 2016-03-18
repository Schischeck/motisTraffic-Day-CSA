#include "motis/rt/handler/addition_handler.h"

#include <iostream>

#include "motis/core/common/util.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_addition(context& ctx, AdditionMessage const* msg) {
  // auto trip = get_trip(ctx, msg->tripId());

  auto id = msg->tripId();
  if (id->base()->stationIdType() != StationIdType_EVA ||
      id->targetStationIdType() != StationIdType_EVA) {

    std::cout << "DS100 !!!!!!!!!!!!!!!" << std::endl;
    return;
  }

  auto eva_nr = id->base()->stationId()->str();
  auto station_id = get_station(ctx.sched, eva_nr)->index;
  auto train_nr = id->base()->trainIndex();
  auto motis_time = unix_to_motistime(ctx.sched, id->base()->scheduledTime());

  auto target_eva_nr = id->targetStationId()->str();
  auto target_station_id = get_station(ctx.sched, target_eva_nr)->index;
  auto target_motis_time =
      unix_to_motistime(ctx.sched, id->targetScheduledTime());
  auto is_arr = (id->base()->type() == EventType_Arrival);
  auto line_id = id->base()->lineId()->str();

  primary_trip_id prim{station_id, train_nr, motis_time};
  secondary_trip_id sec{target_station_id, target_motis_time, is_arr, line_id};
  full_trip_id full = {prim, sec};
  ctx.sched.trip_mem.push_back(make_unique<trip>(full));

  auto const& trp = ctx.sched.trip_mem.back();
  ctx.sched.trips[trp->id.primary].push_back(trp.get());
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
