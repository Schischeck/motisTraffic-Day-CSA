#pragma once

#include <vector>

namespace motis {
namespace reliability {

typedef double PROB_TYPE;

struct probability_distribution {

  void init(std::vector<PROB_TYPE> const& values, int const first_minute);
  void init_one_point(int minute) {}

  int get_first_minute() const { return 0; }
  int get_last_minute() const { return 0; }
};

}  // namespace reliability
}  // namespace motis
