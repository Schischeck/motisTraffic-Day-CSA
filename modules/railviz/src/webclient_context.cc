 #include "motis/railviz/webclient_context.h"

namespace motis {
namespace railviz {

webclient_context::webclient_context(unsigned int id, geometry::box bounds)
{
    this->id = id;
    this->bounds = bounds;

    // set time to current Server Time
    using std::chrono::system_clock;
    system_clock::time_point now = system_clock::now();
    time = time_update_time = system_clock::to_time_t(now);
}

webclient_context::webclient_context(const webclient_context& c)
{
    this->id = c.get_id();
    this->bounds = c.get_bounds();
    set_time( c.get_time() );
}

unsigned int webclient_context::get_id() const
{
    return id;
}

void webclient_context::set_time(time_t time)
{
    using std::chrono::system_clock;
    system_clock::time_point now = system_clock::now();
    time_update_time = system_clock::to_time_t(now);

    this->time = time;
}

std::time_t webclient_context::get_time() const
{
    return this->time;
}

std::time_t webclient_context::get_current_time() const
{
    using std::chrono::system_clock;
    system_clock::time_point now_p = system_clock::now();
    std::time_t now = system_clock::to_time_t(now_p);
    std::time_t run_since = now - time_update_time;

    return time + run_since;
}

void webclient_context::set_bounds(const geometry::box &bounds)
{
    this->bounds = bounds;
}

const geometry::box& webclient_context::get_bounds() const
{
    return bounds;
}

}
}
