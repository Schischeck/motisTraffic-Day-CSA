#include "motis/railviz/mercator.h"

// tooked from http://wiki.openstreetmap.org/wiki/Mercator#C_implementation

namespace motis{
namespace railviz{
static double deg_rad (double ang) {
        return ang * D_R;
}

double merc_x (double lon) {
        return R_MAJOR * deg_rad (lon);
}

double merc_y (double lat) {
        lat = fmin (89.5, fmax (lat, -89.5));
        double phi = deg_rad(lat);
        double sinphi = sin(phi);
        double con = ECCENT * sinphi;
        con = pow((1.0 - con) / (1.0 + con), COM);
        double ts = tan(0.5 * (M_PI * 0.5 - phi)) / con;
        return 0 - R_MAJOR * log(ts);
}

}
}