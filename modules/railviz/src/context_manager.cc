#include "motis/railviz/context_manager.h"

namespace motis {
namespace railviz {

Context& ContextManager::createContext( rtree_box bounds )
{
    Context* c = new Context(nextSessid, bounds);
    contextMap.insert( std::pair<unsigned int, ContextPtr>(nextSessid, ContextPtr(c)) );
    nextSessid++;
    return *c;
}

void ContextManager::removeContext(unsigned int id)
{
    contextMap.erase(id);
}

Context& ContextManager::getContext(unsigned int id)
{
    return *(contextMap.at(id).get());
}

bool ContextManager::contextExists(unsigned int id)
{
    if( contextMap.find(id) == contextMap.end() )
        return false;
    return true;
}

}
}
