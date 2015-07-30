#pragma once

#include <memory>
#include "motis/core/schedule/time.h"

namespace motis {

class light_connection;

namespace railviz {

struct train
{
    unsigned int d_station;
    unsigned int a_station;
    unsigned int route_id;
    const motis::light_connection* light_conenction_;
};

}
}
