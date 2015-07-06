#pragma once

#include <ctime>
#include <map>
#include <memory>

#include "motis/railviz/webclient.h"
#include "motis/module/module.h"

namespace motis {
namespace railviz {

typedef std::unique_ptr<webclient> webclient_ptr;

class webclient_manager
{
public:
    webclient& create_webclient();
    void remove_webclient(motis::module::sid);
    bool webclient_exists(motis::module::sid);
    webclient& get_webclient(motis::module::sid);
private:
    static motis::module::sid next_sessid;
    std::map<motis::module::sid, webclient_ptr> webclient_map;
};

}
}
