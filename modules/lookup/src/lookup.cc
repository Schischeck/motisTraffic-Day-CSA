#include "motis/lookup/lookup.h"

#include "motis/module/context/get_schedule.h"
#include "motis/lookup/error.h"
#include "motis/lookup/lookup_geo_station.h"
#include "motis/lookup/lookup_id_train.h"
#include "motis/lookup/lookup_meta_station.h"
#include "motis/lookup/lookup_station_events.h"
#include "motis/lookup/station_geo_index.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace lookup {

lookup::lookup() : module("Lookup", "lookup") {}
lookup::~lookup() = default;

void lookup::init(registry& r) {
  auto& sched = synced_sched<RO>().sched();
  geo_index_ = std::make_unique<station_geo_index>(sched.stations_);

  r.register_op("/lookup/geo_station_id",
                [this](msg_ptr const& m) { return lookup_station_id(m); });
  r.register_op("/lookup/geo_station",
                [this](msg_ptr const& m) { return lookup_station(m); });
  r.register_op("/lookup/geo_station_batch",
                [this](msg_ptr const& m) { return lookup_stations(m); });
  r.register_op("/lookup/station_events",
                [this](msg_ptr const& m) { return lookup_station_events(m); });
  r.register_op("/lookup/schedule_info",
                [this](msg_ptr const&) { return lookup_schedule_info(); });
  r.register_op("/lookup/id_train",
                [this](msg_ptr const& m) { return lookup_id_train(m); });
  r.register_op("/lookup/meta_station",
                [this](msg_ptr const& m) { return lookup_meta_station(m); });
  r.register_op("/lookup/meta_station_batch",
                [this](msg_ptr const& m) { return lookup_meta_stations(m); });
}

msg_ptr lookup::lookup_station_id(msg_ptr const& msg) const {
  auto req = motis_content(LookupGeoStationIdRequest, msg);

  message_creator b;
  auto response = motis::lookup::lookup_geo_stations_id(b, *geo_index_,
                                                        get_schedule(), req);
  b.create_and_finish(MsgContent_LookupGeoStationResponse, response.Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_station(msg_ptr const& msg) const {
  auto req = motis_content(LookupGeoStationRequest, msg);

  message_creator b;
  auto response = motis::lookup::lookup_geo_stations(b, *geo_index_, req);
  b.create_and_finish(MsgContent_LookupGeoStationResponse, response.Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_stations(msg_ptr const& msg) const {
  auto req = motis_content(LookupBatchGeoStationRequest, msg);

  message_creator b;
  std::vector<Offset<LookupGeoStationResponse>> responses;
  for (auto const& sub_req : *req->requests()) {
    responses.push_back(
        motis::lookup::lookup_geo_stations(b, *geo_index_, sub_req));
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

msg_ptr lookup::lookup_meta_station(msg_ptr const& msg) const {
  auto req = motis_content(LookupMetaStationRequest, msg);

  message_creator b;
  auto& sched = get_schedule();
  b.create_and_finish(
      MsgContent_LookupMetaStationResponse,
      motis::lookup::lookup_meta_station(b, sched, req).Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_meta_stations(msg_ptr const& msg) const {
  auto req = motis_content(LookupBatchMetaStationRequest, msg);

  message_creator b;
  auto& sched = get_schedule();
  std::vector<Offset<LookupMetaStationResponse>> responses;
  for (auto const& r : *req->requests()) {
    responses.push_back(motis::lookup::lookup_meta_station(b, sched, r));
  }
  b.create_and_finish(
      MsgContent_LookupBatchMetaStationResponse,
      CreateLookupBatchMetaStationResponse(b, b.CreateVector(responses))
          .Union());
  return make_msg(b);
}

msg_ptr lookup::lookup_schedule_info() const {
  auto const& sched = get_schedule();
  message_creator b;
  b.create_and_finish(
      MsgContent_LookupScheduleInfoResponse,
      CreateLookupScheduleInfoResponse(b, b.CreateString(sched.name)).Union());
  return make_msg(b);
}

}  // namespace lookup
}  // namespace motis
