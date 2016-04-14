#include "motis/lookup/lookup.h"

#include "motis/module/context/get_schedule.h"
#include "motis/lookup/error.h"
#include "motis/lookup/lookup_id_train.h"
#include "motis/lookup/lookup_station_events.h"
#include "motis/lookup/station_geo_index.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace lookup {

lookup::lookup() : module("Lookup", "lookup") {}
lookup::~lookup() = default;

void lookup::init(registry& r) {
  auto& sched = get_schedule();
  geo_index_ = std::make_unique<station_geo_index>(sched.stations_);

  r.register_op("/lookup/geo_station",
                [this](msg_ptr const& m) { return lookup_station(m); });
  r.register_op("/lookup/geo_station_batch",
                [this](msg_ptr const& m) { return lookup_stations(m); });
  r.register_op("/lookup/station_events",
                [this](msg_ptr const& m) { return lookup_station_events(m); });
  r.register_op("/lookup/id_train",
                [this](msg_ptr const& m) { return lookup_id_train(m); });
}

msg_ptr lookup::lookup_station(msg_ptr const& msg) const {
  auto req = motis_content(LookupGeoStationRequest, msg);

  message_creator b;
  auto response = geo_index_->stations(b, req);
  b.create_and_finish(MsgContent_LookupGeoStationResponse, response.Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_stations(msg_ptr const& msg) const {
  auto req = motis_content(LookupBatchGeoStationRequest, msg);

  message_creator b;
  std::vector<Offset<LookupGeoStationResponse>> responses;
  for (auto const& request : *req->requests()) {
    responses.push_back(geo_index_->stations(b, request));
  }
  b.create_and_finish(
      MsgContent_LookupBatchGeoStationResponse,
      CreateLookupBatchGeoStationResponse(b, b.CreateVector(responses))
          .Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_station_events(msg_ptr const& msg) const {
  auto req = motis_content(LookupStationEventsRequest, msg);

  message_creator b;
  auto& sched = get_schedule();
  auto events = motis::lookup::lookup_station_events(b, sched, req);
  b.create_and_finish(
      MsgContent_LookupStationEventsResponse,
      CreateLookupStationEventsResponse(b, b.CreateVector(events)).Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_id_train(msg_ptr const& msg) const {
  auto req = motis_content(LookupIdTrainRequest, msg);

  message_creator b;
  auto& sched = get_schedule();
  auto train = motis::lookup::lookup_id_train(b, sched, req->trip_id());
  b.create_and_finish(MsgContent_LookupIdTrainResponse,
                      CreateLookupIdTrainResponse(b, train).Union());
  return make_msg(b);
}

}  // namespace lookup
}  // namespace motis
