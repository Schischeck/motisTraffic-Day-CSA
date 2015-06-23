 #include "motis/railviz/context.h"

namespace motis{
namespace railviz{

Context::Context(unsigned int id, RTreeBox bounds)
{
    this->id = id;
    this->bounds = bounds;

    // set time to current Server Time
    using std::chrono::system_clock;
    system_clock::time_point now = system_clock::now();
    time = timeUpdateTime = system_clock::to_time_t(now);
}

Context::Context(const Context& c)
{
    this->id = c.getID();
    this->bounds = c.getBounds();
    setTime( c.getTime() );
}

unsigned int Context::getID() const
{
    return id;
}

void Context::setTime(time_t time)
{
    using std::chrono::system_clock;
    system_clock::time_point now = system_clock::now();
    timeUpdateTime = system_clock::to_time_t(now);

    this->time = time;
}

std::time_t Context::getTime() const
{
    return this->time;
}

std::time_t Context::getCurrentTime() const
{
    using std::chrono::system_clock;
    system_clock::time_point nowP = system_clock::now();
    std::time_t now = system_clock::to_time_t(nowP);
    std::time_t runSince = now - timeUpdateTime;

    return time + runSince;
}

void Context::setBounds(const RTreeBox &bounds)
{
    this->bounds = bounds;
}

const RTreeBox& Context::getBounds() const
{
    return bounds;
}

}
}
