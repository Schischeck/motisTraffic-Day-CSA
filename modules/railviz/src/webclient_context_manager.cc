#include "motis/railviz/webclient_context_manager.h"

namespace motis {
namespace railviz {

unsigned int webclient_context_manager::next_sessid = 0;

webclient_context& webclient_context_manager::create_webclient_context()
{
    webclient_context* c = new webclient_context(next_sessid);
    webclient_context_map.insert( std::pair<unsigned int, webclient_context_ptr>(next_sessid, webclient_context_ptr(c)) );
    next_sessid++;
    return *c;
}

void webclient_context_manager::remove_webclient_context(unsigned int id)
{
    webclient_context_map.erase(id);
}

webclient_context& webclient_context_manager::get_webclient_context(unsigned int id)
{
    return *(webclient_context_map.at(id).get());
}

bool webclient_context_manager::webclient_context_exists(unsigned int id)
{
    if( webclient_context_map.find(id) == webclient_context_map.end() )
        return false;
    return true;
}

}
}
