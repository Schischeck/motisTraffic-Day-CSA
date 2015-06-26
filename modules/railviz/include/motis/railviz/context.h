#pragma once

#include <ctime>
#include <chrono>
#include "motis/railviz/rtree.h"

namespace motis {
namespace railviz {

class Context
{
public:
    Context( unsigned int id, rtree_box bounds );
    Context( const Context& );

    unsigned int getID() const;

    void setTime(std::time_t time);
    std::time_t getTime() const;
    std::time_t getCurrentTime() const;

    void setBounds( const rtree_box& bounds );
    const rtree_box& getBounds() const;
private:
    unsigned int id;

    std::time_t timeUpdateTime; // time when the init-time was set
    std::time_t time;


    rtree_box bounds;

};

}
}
