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
    webclient( unsigned int id, geo::box bounds );
    webclient( const webclient& );

    unsigned int get_id() const;

    void set_time(std::time_t time);
    std::time_t get_time() const;
    std::time_t get_current_time() const;

    void set_bounds( const geo::box& bounds );
    const geo::box& get_bounds() const;
private:
    unsigned int id;

    std::time_t time_update_time; // time when the init-time was set
    std::time_t time;


    geo::box bounds;

};

}
}
