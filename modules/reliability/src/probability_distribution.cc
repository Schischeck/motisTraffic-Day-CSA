#include "motis/reliability/probability_distribution.h"

#include <algorithm>
#include <cassert>

namespace motis {
namespace reliability {

void probability_distribution::init(
    std::vector<probability> const& probabilities, int const first_minute) {
  probabilities_.clear();

  /* determine the left bound (ignore all values smaller than
   * THRESHOLD_SMALL_VALUES) */
  probability error = 0.0;  // sum of all ignored values
  int idx_first_value = 0;
  while (idx_first_value + 1 < probabilities.size() &&
         probabilities[idx_first_value] < THRESHOLD_SMALL_VALUES) {
    error += probabilities[idx_first_value];
    idx_first_value++;
  }
  first_minute_ = first_minute + idx_first_value;

  /* determine the right bound (ignore all values smaller than
   * THRESHOLD_SMALL_VALUES) */
  unsigned int idx_last_value = probabilities.size() - 1;
  while (idx_last_value > idx_first_value &&
         probabilities[idx_last_value] < THRESHOLD_SMALL_VALUES) {
    error += probabilities[idx_last_value];
    idx_last_value--;
  }

  if (error > 0.005) {
    std::cout << "\nWarning(probability_distribution::init): error too large: "
              << error << std::endl;
  }

  // error is added to the probability of first-minute
  probability cumulative_probability = error;
  for (unsigned int i = idx_first_value; i <= idx_last_value; i++) {
    assert(!smaller(probabilities[i], 0.0));
    cumulative_probability += probabilities[i];
    probabilities_.push_back(cumulative_probability);
  }
}

void probability_distribution::init_one_point(int minute, probability prob) {
  probabilities_.clear();
  probabilities_.push_back(prob);
  first_minute_ = minute;
}

int probability_distribution::last_minute() const {
  return (first_minute_ + probabilities_.size()) - 1;
}

probability probability_distribution::probability_smaller(
    int const minute) const {
  return probability_smaller_equal(minute - 1);
}

probability probability_distribution::probability_smaller_equal(
    int const minute) const {
  if (minute < first_minute_) {
    return 0.0;
  }
  if (minute > last_minute()) {
    return sum();
  }
  return probabilities_[probability_index(minute)];
}

probability probability_distribution::probability_equal(
    int const minute) const {
  if (minute < first_minute_ || minute > last_minute()) {
    return 0.0;
  }
  if (minute == first_minute_) {
    return probabilities_[0];
  }

  return probabilities_[probability_index(minute)] -
         probabilities_[probability_index(minute - 1)];
}

probability probability_distribution::probability_greater_equal(
    int const minute) const {
  if (minute < first_minute_) {
    return sum();
  }
  if (minute > last_minute()) {
    return 0.0;
  }

  return sum() - probability_smaller(minute);
}

probability probability_distribution::probability_greater(
    int const minute) const {
  return probability_greater_equal(minute + 1);
}

probability probability_distribution::sum() const {
  if (probabilities_.size() == 0) return 0.0;
  return probabilities_[probabilities_.size() - 1];
}

void probability_distribution::get_probabilities(
    std::vector<probability>& probabilities) const {
  for (int i = first_minute_; i <= last_minute(); i++)
    probabilities.push_back(probability_equal(i));
}

std::ostream& operator<<(std::ostream& os,
                         probability_distribution const& distribution) {
  os << "[sum=" << distribution.sum()
     << " first-min=" << distribution.first_minute()
     << " last-min=" << distribution.last_minute() << " values=";
  for (int i = distribution.first_minute(); i <= distribution.last_minute();
       i++) {
    os << distribution.probability_equal(i);
    if (i + 1 <= distribution.last_minute()) {
      os << ",";
    }
  }
  os << "]";
  return os;
}

const double probability_distribution::THRESHOLD_SMALL_VALUES = 0.00001;

}  // namespace reliability
}  // namespace motis
