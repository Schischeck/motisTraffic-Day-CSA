#include "motis/railviz/geometry.h"

namespace motis {
namespace railviz {
namespace geometry {
std::ostream& operator<< (std::ostream& stream, const point& p)
{
    stream << "(" << p.lat << "|" << p.lng << ")";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const box& box)
{
    stream << "[" << box.p1 << ", " << box.p2 << "]";
    return stream;
}
}
}
}
