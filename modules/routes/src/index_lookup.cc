#include "motis/routes/index_lookup.h"

#include "motis/core/common/transform_to_vec.h"

namespace motis {
namespace routes {

index_lookup::index_lookup(lookup_table const& lt) : lookup_table_(lt) {}

std::string index_lookup::lookup(RoutesStationSeqRequest const* req) {
  auto const& lookup = *lookup_table_.get()->indices();
  auto it =
      std::find_if(std::begin(lookup), std::end(lookup), [&](auto const& p) {
        return p->station_ids() == req->station_ids() &&
               std::find(p->classes()->begin(), p->classes()->end(),
                         req->clasz()) != p->classes()->end();

      });

  if (it == std::end(lookup)) {
    return "";
  }
  return std::to_string(it->index());
}

}  // namespace routes
}  // namespace motis
