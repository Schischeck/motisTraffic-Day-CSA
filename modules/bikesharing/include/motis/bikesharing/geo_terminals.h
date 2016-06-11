#pragma once

#include "motis/module/message.h"
#include "motis/bikesharing/database.h"
#include "motis/bikesharing/geo_index.h"

namespace motis {
namespace bikesharing {

module::msg_ptr geo_terminals(database const&, geo_index const&,
                              BikesharingGeoTerminalsRequest const*);

}  // namespace bikesharing
}  // namespace motis
