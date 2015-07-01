#pragma once

#include <ctime>
#include <chrono>
#include "motis/railviz/geometry.h"

namespace motis {
namespace railviz {

class webclient_context
{
public:
    webclient_context( unsigned int id );
    webclient_context( unsigned int id, geometry::box bounds );
    webclient_context( const webclient_context& );

    unsigned int get_id() const;

    void set_time(std::time_t time);
    std::time_t get_time() const;
    std::time_t get_current_time() const;

    void set_bounds( const geometry::box& bounds );
    const geometry::box& get_bounds() const;
private:
    unsigned int id;

    std::time_t time_update_time; // time when the init-time was set
    std::time_t time;


    geometry::box bounds;

};

}
}
