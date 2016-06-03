#include <algorithm>

#include "gtest/gtest.h"

#include "motis/reliability/intermodal/hotels.h"

namespace motis {
namespace reliability {
namespace intermodal {

TEST(reliability_hotels, parse) {
  std::vector<hotel> hotels;
  parse_hotels("modules/reliability/resources/hotels.csv", hotels);
  ASSERT_FALSE(hotels.empty());
  ASSERT_NE(hotels.end(),
            std::find_if(hotels.begin(), hotels.end(), [](auto const& h) {
              return h.station_ == "8000013";
            }));
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
