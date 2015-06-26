#pragma once

#include <ctime>
#include <map>

#include "motis/railviz/rtree.h"
#include "motis/railviz/context.h"

namespace motis {
namespace railviz {

typedef std::unique_ptr<Context> ContextPtr;

class ContextManager
{
public:
    Context& createContext(geometry::box);
    void removeContext(unsigned int id);
    bool contextExists(unsigned int id);
    Context& getContext(unsigned int id);
private:
    unsigned int nextSessid;
    std::map<unsigned int, ContextPtr> contextMap;
};

}
}
