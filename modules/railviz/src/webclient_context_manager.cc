#include "motis/railviz/webclient_manager.h"

namespace motis {
namespace railviz {

unsigned int webclient_manager::next_sessid = 0;

webclient& webclient_manager::create_webclient()
{
    webclient* c = new webclient(next_sessid);
    webclient_map.insert( std::pair<unsigned int, webclient_ptr>(next_sessid, webclient_ptr(c)) );
    next_sessid++;
    return *c;
}

void webclient_manager::remove_webclient(unsigned int id)
{
    webclient_map.erase(id);
}

webclient& webclient_manager::get_webclient(unsigned int id)
{
    return *(webclient_map.at(id).get());
}

bool webclient_manager::webclient_exists(unsigned int id)
{
    if( webclient_map.find(id) == webclient_map.end() )
        return false;
    return true;
}

}
}
