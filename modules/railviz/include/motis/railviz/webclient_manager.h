#pragma once

#include <ctime>
#include <map>
#include <memory>

#include "motis/railviz/webclient.h"

namespace motis {
namespace railviz {

typedef std::unique_ptr<webclient> webclient_ptr;

class webclient_manager
{
public:
    webclient& create_webclient();
    void remove_webclient(unsigned int id);
    bool webclient_exists(unsigned int id);
    webclient& get_webclient(unsigned int id);
private:
    static unsigned int next_sessid;
    std::map<unsigned int, webclient_ptr> webclient_map;
};

}
}
