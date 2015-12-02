#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "motis/core/schedule/time.h"

namespace motis {
namespace reliability {

typedef double probability;

struct probability_distribution {
  probability_distribution() : first_minute_(0) {}

  void init(std::vector<probability> const& probabilities,
            int const first_minute);
  void init(probability_distribution const& other);
  void init_one_point(int const minute, probability const prob);

  bool empty() const { return probabilities_.size() == 0; }

  int first_minute() const { return first_minute_; }
  int last_minute() const;

  probability probability_smaller(int const minute) const;
  probability probability_smaller_equal(int const minute) const;
  probability probability_equal(int const minute) const;
  probability probability_greater_equal(int const minute) const;
  probability probability_greater(int const minute) const;
  probability sum() const;

  /* insert all probabilities in to the vector 'probabilities' */
  template <typename Prob_Type>
  void get_probabilities(std::vector<Prob_Type>& probabilities) const {
    for (int i = first_minute_; i <= last_minute(); i++)
      probabilities.push_back((Prob_Type)probability_equal(i));
  }

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

inline int timestamp_to_delay(time const& scheduled_time,
                              time const& delayed_time) {
  return delayed_time - scheduled_time;
}

inline bool equal(probability const& a, probability const& b) {
  // http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
  return (a == b || std::abs(a - b) < 0.000001
          /*std::abs(a - b) < std::numeric_limits<probability>::epsilon()*/
          /* * std::abs(a + b) * 6*/);
}

inline bool smaller(probability const& a, probability const& b) {
  return (a < b && !equal(a, b));
}

inline bool smaller_equal(probability const& a, probability const& b) {
  return (a <= b || equal(a, b));
}

inline bool greater(probability const& a, probability const& b) {
  return (a > b && !equal(a, b));
}

inline bool greater_equal(probability const& a, probability const& b) {
  return (a >= b || equal(a, b));
}

inline bool operator==(probability_distribution const& lhs,
                       probability_distribution const& rhs) {
  if (lhs.first_minute() != rhs.first_minute() ||
      lhs.last_minute() != rhs.last_minute() || !equal(lhs.sum(), rhs.sum())) {
    return false;
  }
  for (int d = lhs.first_minute(); d <= lhs.last_minute(); d++) {
    if (!equal(lhs.probability_smaller_equal(d),
               rhs.probability_smaller_equal(d))) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(probability_distribution const& lhs,
                       probability_distribution const& rhs) {
  return !(lhs == rhs);
}

}  // namespace reliability
}  // namespace motis
