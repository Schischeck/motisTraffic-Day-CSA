#pragma once

#include <math.h>

namespace motis{
namespace railviz{

// tooked from http://wiki.openstreetmap.org/wiki/Mercator#C_implementation

/*
 * Mercator transformation
 * accounts for the fact that the earth is not a sphere, but a spheroid
 */
#define D_R (M_PI / 180.0)
#define R_D (180.0 / M_PI)
#define R_MAJOR 6378137.0
#define R_MINOR 6356752.3142
#define RATIO (R_MINOR/R_MAJOR)
#define ECCENT (sqrt(1.0 - (RATIO * RATIO)))
#define COM (0.5 * ECCENT)

static double deg_rad (double ang);

double merc_x (double lon);

double merc_y (double lat);

}
}
