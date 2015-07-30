#pragma once

#include <ctime>
#include <chrono>
#include "motis/railviz/geo.h"

namespace motis {
namespace railviz {

class timetable
{
public:
    timetable(  );
    timetable( const timetable& );

    unsigned int id;
    std::time_t time;
    geo::box bounds;

};

}
}
