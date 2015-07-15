#pragma once

#include <ctime>
#include <chrono>
#include "motis/railviz/geo.h"

namespace motis {
namespace railviz {

class webclient
{
public:
    webclient( unsigned int id );
    webclient( const webclient& );

    unsigned int id;
    std::time_t time;
    geo::box bounds;

};

}
}
