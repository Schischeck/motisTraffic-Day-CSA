#pragma once

#include <ctime>

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/date_manager.h"

namespace motis {
namespace railviz {

class date_converter
{
public:
    date_converter( const date_manager& mgr );
    std::time_t convert(motis::time const& t) const;
private:
    const date_manager& mgr;

};

}
}
