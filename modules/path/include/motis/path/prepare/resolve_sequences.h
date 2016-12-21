#pragma once

#include <vector>

#include "motis/path/db/db_builder.h"
#include "motis/path/prepare/routing/path_routing.h"
#include "motis/path/prepare/schedule/station_sequences.h"

namespace motis {
namespace path {

void resolve_sequences(std::vector<station_seq> const&, path_routing&,
                       db_builder&);

}  // namespace path
}  // namespace motis
