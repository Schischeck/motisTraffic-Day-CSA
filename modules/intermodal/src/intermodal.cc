#include "../include/motis/intermodal/intermodal.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/loader/util.h"
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

struct intermodal::impl {
  explicit impl(std::vector<station_ptr> const& stations)
      : station_index_(stations) {}

  std::vector<std::vector<coordinate>> reachable(coordinate const& start) {
    auto car_stations = station_index_.stations(start.lat, start.lng, 15000);
    auto bike_stations = station_index_.stations(start.lat, start.lng, 5000);
    auto walk_stations = station_index_.stations(start.lat, start.lng, 700);

    return {transform_to_vec(begin(car_stations), end(car_stations),
                             [](station const* s) {
                               return coordinate{s->width, s->length};
                             }),
            transform_to_vec(begin(bike_stations), end(bike_stations),
                             [](station const* s) {
                               return coordinate{s->width, s->length};
                             }),
            transform_to_vec(begin(walk_stations), end(walk_stations),
                             [](station const* s) {
                               return coordinate{s->width, s->length};
                             })};
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
  auto req = msg->content<IntermodalRequest const*>();
  auto reachable = impl_->reachable(coordinate{req->lat(), req->lng()});

  MessageCreator fbb;
  fbb.CreateAndFinish(
      MsgContent_IntermodalResponse,
      CreateIntermodalResponse(
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

}  // namespace intermodal
}  // namespace motis
