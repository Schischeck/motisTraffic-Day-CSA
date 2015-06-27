#pragma once

#include <ctime>
#include <map>
#include <memory>

#include "motis/railviz/webclient_context.h"
#include "motis/railviz/geometry.h"

namespace motis {
namespace railviz {

typedef std::unique_ptr<webclient_context> webclient_context_ptr;

class webclient_context_manager
{
public:
    webclient_context& create_webclient_context(geometry::box);
    void remove_webclient_context(unsigned int id);
    bool webclient_context_exists(unsigned int id);
    webclient_context& get_webclient_context(unsigned int id);
private:
    unsigned int next_sessid;
    std::map<unsigned int, webclient_context_ptr> webclient_context_map;
};

}
}
