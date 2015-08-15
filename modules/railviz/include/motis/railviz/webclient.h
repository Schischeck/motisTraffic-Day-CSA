#pragma once

#include <ctime>

#include "motis/railviz/geo.h"

namespace motis {
namespace railviz {

class webclient {
 public:
  webclient(unsigned int id) : id(id), time(0), bounds({{0, 0}, {0, 0}}) {}

  unsigned int id;
  std::time_t time;
  geo::box bounds;
};

}  // namespace railviz
}  // namespace motis
