#pragma once

#include <vector>

#include "motis/path/db/db_builder.h"
#include "motis/path/prepare/routing/routing.h"
#include "motis/path/prepare/schedule/station_sequences.h"

namespace motis {
namespace path {

void resolve_sequences(std::vector<station_seq> const&, routing&, db_builder&);

}  // namespace path
}  // namespace motis
