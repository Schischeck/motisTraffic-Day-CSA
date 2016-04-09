#include "motis/eval/comparator/response.h"

#include <numeric>

#include "motis/loader/util.h"

using namespace motis::routing;
using motis::loader::transform_to_vec;

namespace motis {
namespace eval {
namespace comparator {

unsigned travel_time(Connection const* c) {
  return c->stops()->Get(c->stops()->size() - 1)->arrival()->time() -
         c->stops()->Get(0)->departure()->time();
}

unsigned transfers(Connection const* c) {
  return std::accumulate(
      std::begin(*c->stops()), std::end(*c->stops()), 0,
      [](int acc, Stop const* s) { return s->interchange() ? acc + 1 : acc; });
}

unsigned price(Connection const*) { return 0; }

response::response(RoutingResponse const* r)
    : connections(transform_to_vec(
          std::begin(*r->connections()), std::end(*r->connections()),
          [](Connection const* c) {
            return std::make_tuple(travel_time(c), transfers(c), price(c));
          })) {}

}  // namespace comparator
}  // namespace eval
}  // namespace motis