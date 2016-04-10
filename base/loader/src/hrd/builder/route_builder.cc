#include "motis/loader/hrd/builder/route_builder.h"

#include "motis/core/common/get_or_create.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

Offset<Route> route_builder::get_or_create_route(
    std::vector<hrd_service::stop> const& stops, station_builder& sb,
    FlatBufferBuilder& fbb) {
  auto events = transform_to_vec(
      begin(stops), end(stops), [&](hrd_service::stop const& s) {
        return stop_restrictions{s.eva_num_, s.dep_.in_out_allowed_,
                                 s.arr_.in_out_allowed_};
      });
  return get_or_create(routes_, events, [&]() {
    return CreateRoute(fbb,
                       fbb.CreateVector(transform_to_vec(
                           begin(events), end(events),
                           [&](stop_restrictions const& sr) {
                             return sb.get_or_create_station(sr.eva_num_, fbb);
                           })),
                       fbb.CreateVector(transform_to_vec(
                           begin(events), end(events),
                           [](stop_restrictions const& sr) -> uint8_t {
                             return sr.entering_allowed_ ? 1 : 0;
                           })),
                       fbb.CreateVector(transform_to_vec(
                           begin(events), end(events),
                           [](stop_restrictions const& sr) -> uint8_t {
                             return sr.leaving_allowed_ ? 1 : 0;
                           })));
  });
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
