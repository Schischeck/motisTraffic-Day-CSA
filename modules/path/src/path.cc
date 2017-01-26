#include "motis/path/path.h"

#include <memory>

#include "parser/util.h"

#include "geo/polyline.h"

#include "utl/to_vec.h"

#include "motis/core/common/logging.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"

#include "motis/path/db/kv_database.h"
#include "motis/path/lookup_index.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;
using namespace motis::logging;

// --

#include "util/coordinate.hpp"  // osrm
#include "util/json_container.hpp"  // osrm
#include "util/json_util.hpp"  // osrm

#include "engine/douglas_peucker.hpp"  // osrm

using namespace osrm::util;
using namespace osrm::engine;

FloatCoordinate make_coord(geo::latlng const& pos) {
  return FloatCoordinate{FloatLongitude{pos.lng_}, FloatLatitude{pos.lat_}};
}

geo::latlng make_latlng(FloatCoordinate const& coord) {
  return {static_cast<double>(coord.lat), static_cast<double>(coord.lon)};
}

// --

namespace motis {
namespace path {

path::path() : module("Path", "path") {
  string_param(database_path_, "pathdb", "db", "/path/to/pathdb");
}

path::~path() = default;

void path::init(registry& r) {
  db_ = load_db(database_path_);

  if (auto buf = db_->try_get("__index")) {
    lookup_ = std::make_unique<lookup_index>(*buf);
  } else {
    LOG(warn) << "pathdb not found!";
    lookup_ = std::make_unique<lookup_index>();
  }

  r.register_op("/path/station_seq",
                [this](msg_ptr const& m) { return station_seq_path(m); });
  r.register_op("/path/id_train",
                [this](msg_ptr const& m) { return id_train_path(m); });
}

msg_ptr path::station_seq_path(msg_ptr const& msg) const {
  auto req = motis_content(PathStationSeqRequest, msg);

  return get_response(
      lookup_->find({utl::to_vec(*req->station_ids(),
                                 [](auto const& s) { return s->str(); }),
                     req->clasz()}),
      req->zoom_level(), req->debug_info());
}

msg_ptr path::id_train_path(msg_ptr const& msg) const {
  auto const& req = motis_content(PathIdTrainRequest, msg);
  auto const& sched = get_schedule();
  auto const& t = req->trip_id();
  auto const& trp = get_trip(sched, t->station_id()->str(), t->train_nr(),
                             t->time(), t->target_station_id()->str(),
                             t->target_time(), t->line_id()->str());

  auto const seq = utl::to_vec(access::stops(trp), [&sched](auto const& stop) {
    return stop.get_station(sched).eva_nr_;
  });
  auto const clasz = trip_section{trp, 0}.fcon().clasz_;
  return get_response(lookup_->find({seq, clasz}), req->zoom_level(),
                      req->debug_info());
}

msg_ptr path::get_response(std::string const& index, int const zoom_level,
                           bool const debug_info) const {
  auto buf = db_->get(index);
  auto original_msg = std::make_shared<message>(buf.size(), buf.c_str());
  auto original = motis_content(PathSeqResponse, original_msg);

  if (zoom_level == -1 && debug_info) {
    return original_msg;
  }

  message_creator mc;

  auto const station_ids =
      utl::to_vec(*original->station_ids(),
                  [&mc](auto const& e) { return mc.CreateString(e->c_str()); });
  auto const classes = utl::to_vec(*original->classes());

  auto const copy = [&mc, &original] {
    return utl::to_vec(*original->segments(), [&mc](auto const& segment) {
      auto const polyline = utl::to_vec(*segment->coordinates());
      return CreatePolyline(mc, mc.CreateVector(polyline));
    });
  };

  auto const simplify = [&mc, &original, &zoom_level] {
    verify(zoom_level <= 20 && zoom_level > -1, "invalid zoom level");
    return utl::to_vec(
        *original->segments(), [&mc, &zoom_level](auto const& segment) {
          auto const original_polyline =
              geo::deserialize(utl::to_vec(*segment->coordinates()));

          auto const osrm_coords = utl::to_vec(
              original_polyline,
              [](auto const& c) -> Coordinate { return make_coord(c); });

          auto const simplified_osrm_coords =
              douglasPeucker(osrm_coords, zoom_level);

          auto const simplified_polyline = utl::to_vec(
              simplified_osrm_coords,
              [](auto const& osrm_coord) { return make_latlng(osrm_coord); });

          return CreatePolyline(
              mc, mc.CreateVector(geo::serialize(simplified_polyline)));
        });
  };

  auto const segments = (zoom_level == -1) ? copy() : simplify();

  // either polylines are simplified or debug_info == false
  auto const source_infos = std::vector<Offset<PathSourceInfo>>{};

  mc.create_and_finish(
      MsgContent_PathSeqResponse,
      CreatePathSeqResponse(mc, mc.CreateVector(station_ids),
                            mc.CreateVector(classes), mc.CreateVector(segments),
                            mc.CreateVector(source_infos))
          .Union());
  return make_msg(mc);
}

}  // namespace path
}  // namespace motis
