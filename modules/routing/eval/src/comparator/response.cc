#include "motis/eval/comparator/response.h"

#include <numeric>

#include "motis/core/common/transform_to_set.h"

using namespace motis::routing;

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
             [](int acc, Stop const* s) { return s->exit() ? acc + 1 : acc; }) -
         1;
}

unsigned price(Connection const*) { return 0; }

response::response(RoutingResponse const* r)
    : connections_(transform_to_set(*r->connections(), [](Connection const* c) {
        return std::make_tuple(travel_time(c), transfers(c), price(c));
      })) {}

}  // namespace comparator
}  // namespace eval
}  // namespace motis
