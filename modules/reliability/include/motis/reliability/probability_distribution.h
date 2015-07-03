#pragma once

namespace motis {
namespace reliability {

struct probability_distribution {

  int get_first_minute() const { return 0; }
  int get_last_minute() const { return 0; }
};

}  // namespace reliability
}  // namespace motis
