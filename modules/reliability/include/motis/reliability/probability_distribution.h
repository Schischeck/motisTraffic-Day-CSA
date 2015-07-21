#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace motis {
namespace reliability {

typedef double probability;

struct probability_distribution {

  void init(std::vector<probability> const& values, int const first_minute);
  void init_one_point(int const minute, probability const prob);

  int first_minute() const { return first_minute_; }
  int last_minute() const;

  probability probability_smaller(int const minute) const;
  probability probability_smaller_equal(int const minute) const;
  probability probability_equal(int const minute) const;
  probability probability_greater_equal(int const minute) const;
  probability probability_greater(int const minute) const;
  probability sum() const;

  friend std::ostream& operator<<(std::ostream& os,
                                  probability_distribution const& distribution);

private:
  inline unsigned int probability_index(int minute) const {
    return minute - first_minute_;
  }

  std::vector<probability> probabilities_;
  int first_minute_;

  static const double THRESHOLD_SMALL_VALUES;
};

inline bool equal(probability const& a, probability const& b) {
  // http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
  return (a == b ||
          std::abs(a - b) < std::numeric_limits<probability>::epsilon() *
                                std::abs(a + b) * 6);
}

inline bool smaller(probability const& a, probability const& b) {
  return (a < b && !equal(a, b));
}

inline bool greater(probability const& a, probability const& b) {
  return (a > b && !equal(a, b));
}

}  // namespace reliability
}  // namespace motis
