 #include "motis/railviz/webclient.h"

namespace motis {
namespace railviz {

webclient::webclient(unsigned int id)
{
    this->id = id;
    this->bounds = {{0,0},{0,0}};
    this->time = 0;
}

webclient::webclient(const webclient& c)
{
    this->id = c.id;
    this->bounds = c.bounds;
    this->time = c.time;
}

}
}
