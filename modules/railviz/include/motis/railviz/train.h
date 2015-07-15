#pragma once

#include <memory>
#include "motis/core/schedule/time.h"

namespace motis {
namespace railviz {

struct train
{
    unsigned int d_station;
    unsigned int a_station;
    motis::time d_time;
    motis::time a_time;
};

}
}
