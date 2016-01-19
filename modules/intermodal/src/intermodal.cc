#include "../include/motis/intermodal/intermodal.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/loader/util.h"
#include "motis/intermodal/error.h"
#include "motis/intermodal/station_geo_index.h"
#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
using motis::loader::transform_to_vec;
namespace po = boost::program_options;

namespace motis {
namespace intermodal {

struct coordinate {
  double lat, lng;
};

template <typename T>
constexpr T identity(T&& v) {
  return std::forward<T>(v);
}

struct intermodal::impl {
  explicit impl(std::vector<station_ptr> const& stations)
      : station_index_(stations) {}

  std::vector<std::vector<coordinate>> reachable(coordinate const& pos) const {
    auto const station_to_coordinates = [](station const* s) {
      return coordinate{s->width, s->length};
    };

    return {close_stations(pos.lat, pos.lng, 15000, station_to_coordinates),
            close_stations(pos.lat, pos.lng, 5000, station_to_coordinates),
            close_stations(pos.lat, pos.lng, 700, station_to_coordinates)};
  }

  template <typename F>
  std::vector<typename std::result_of<F(station const*)>::type> close_stations(
      double const lat, double const lng, double const radius,
      F func = identity) const {
    auto stations = station_index_.stations(lat, lng, radius);
    return transform_to_vec(begin(stations), end(stations), func);
  }

  station_geo_index station_index_;
};

intermodal::intermodal() {}

intermodal::~intermodal() {}

po::options_description intermodal::desc() {
  po::options_description desc("Intermodal Module");
  return desc;
}

void intermodal::print(std::ostream&) const {}

void intermodal::init() {
  impl_ = make_unique<impl>(synced_sched<RO>().sched().stations);
}

void intermodal::on_msg(msg_ptr msg, sid, callback cb) {
  auto content_type = msg->msg_->content_type();

  if (content_type == MsgContent_IntermodalRoutingRequest) {
    auto req = msg->content<IntermodalRoutingRequest const*>();
    return handle_routing_request(req, cb);

  } else if (content_type == MsgContent_IntermodalGeoIndexRequest) {
    auto req = msg->content<IntermodalGeoIndexRequest const*>();
    return handle_geo_index_request(req, cb);
  }

  return cb({}, error::not_implemented);
}

void intermodal::handle_routing_request(IntermodalRoutingRequest const* req,
                                        callback cb) {
  auto reachable = impl_->reachable(coordinate{req->lat(), req->lng()});

  MessageCreator fbb;
  fbb.CreateAndFinish(
      MsgContent_IntermodalRoutingResponse,
      CreateIntermodalRoutingResponse(
          fbb, fbb.CreateVector(transform_to_vec(
                   begin(reachable), end(reachable),
                   [&](std::vector<coordinate> const& stations) {
                     std::vector<Coordinate> station_coordinates;
                     station_coordinates.reserve(stations.size());
                     std::transform(begin(stations), end(stations),
                                    std::back_inserter(station_coordinates),
                                    [](coordinate const& c) {
                                      return Coordinate(c.lat, c.lng);
                                    });
                     return CreateMode(
                         fbb, fbb.CreateVectorOfStructs(station_coordinates));
                   })))
          .Union());
  return cb(make_msg(fbb), boost::system::error_code());
}

void intermodal::handle_geo_index_request(IntermodalGeoIndexRequest const* req,
                                          callback cb) {
  MessageCreator fbb;
  auto const storeStation = [&fbb](station const* s) {
    return CreateStation(fbb, fbb.CreateString(s->name),
                         fbb.CreateString(s->eva_nr), s->width, s->length);
  };

  std::vector<Offset<StationList>> list;
  for (auto const& c : *req->coordinates()) {
    list.push_back(CreateStationList(
        fbb, fbb.CreateVector(impl_->close_stations(
                 c->lat(), c->lng(), c->radius(), storeStation))));
  }

  fbb.CreateAndFinish(
      MsgContent_IntermodalGeoIndexResponse,
      CreateIntermodalGeoIndexResponse(fbb, fbb.CreateVector(list)).Union());

  return cb(make_msg(fbb), error::ok);
}

}  // namespace intermodal
}  // namespace motis
