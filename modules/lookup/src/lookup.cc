#include "motis/lookup/lookup.h"

#include "boost/program_options.hpp"

#include "motis/core/common/util.h"
#include "motis/lookup/error.h"
#include "motis/lookup/lookup_id_train.h"
#include "motis/lookup/lookup_station_events.h"
#include "motis/lookup/station_geo_index.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace lookup {

lookup::lookup() = default;
lookup::~lookup() = default;

po::options_description lookup::desc() {
  po::options_description desc("lookup Module");
  return desc;
}

void lookup::print(std::ostream&) const {}

void lookup::init() {
  auto lock = synced_sched<RO>();
  geo_index_ = make_unique<station_geo_index>(lock.sched().stations);
}

void lookup::on_msg(msg_ptr msg, sid, callback cb) {
  try {
    auto type = msg->content_type();
    switch (type) {
      case MsgContent_LookupGeoStationRequest: {
        auto req = msg->content<LookupGeoStationRequest const*>();
        return lookup_station(req, cb);
      }
      case MsgContent_LookupBatchGeoStationRequest: {
        auto req = msg->content<LookupBatchGeoStationRequest const*>();
        return lookup_stations(req, cb);
      }
      case MsgContent_LookupStationEventsRequest: {
        auto req = msg->content<LookupStationEventsRequest const*>();
        return lookup_station_events(req, cb);
      }
      case MsgContent_LookupIdTrainRequest: {
        auto req = msg->content<LookupIdTrainRequest const*>();
        return lookup_id_train(req, cb);
      }
      default: return cb({}, error::not_implemented);
    }
  } catch (boost::system::system_error const& e) {
    return cb({}, e.code());
  }
}

void lookup::lookup_station(LookupGeoStationRequest const* req,
                            callback cb) const {
  MessageCreator b;
  auto response = geo_index_->stations(b, req);
  b.CreateAndFinish(MsgContent_LookupGeoStationResponse, response.Union());
  return cb(make_msg(b), error::ok);
}

void lookup::lookup_stations(LookupBatchGeoStationRequest const* req,
                             motis::module::callback cb) const {
  MessageCreator b;
  std::vector<Offset<LookupGeoStationResponse>> responses;
  for (auto const& request : *req->requests()) {
    responses.push_back(geo_index_->stations(b, request));
  }
  b.CreateAndFinish(
      MsgContent_LookupBatchGeoStationResponse,
      CreateLookupBatchGeoStationResponse(b, b.CreateVector(responses))
          .Union());
  return cb(make_msg(b), error::ok);
}

void lookup::lookup_station_events(LookupStationEventsRequest const* req,
                                   callback cb) {
  MessageCreator b;
  auto lock = synced_sched<schedule_access::RO>();
  auto events = motis::lookup::lookup_station_events(b, lock.sched(), req);
  b.CreateAndFinish(
      MsgContent_LookupStationEventsResponse,
      CreateLookupStationEventsResponse(b, b.CreateVector(events)).Union());
  return cb(make_msg(b), error::ok);
}

void lookup::lookup_id_train(LookupIdTrainRequest const* req, callback cb) {
  MessageCreator b;
  auto lock = synced_sched<schedule_access::RO>();
  auto train = motis::lookup::lookup_id_train(b, lock.sched(), req->id_event());
  b.CreateAndFinish(MsgContent_LookupIdTrainResponse,
                    CreateLookupIdTrainResponse(b, train).Union());
  return cb(make_msg(b), error::ok);
}

}  // namespace lookup
}  // namespace motis
