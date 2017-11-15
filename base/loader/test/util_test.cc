#include <cinttypes>
#include <cstring>
#include <functional>

#include "gtest/gtest.h"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/core/common/date_time_util.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_util, bitset_to_string_and_back) {
  std::string bit_string = "0101010100101010";
  std::bitset<16> before(bit_string);

  ASSERT_TRUE(deserialize_bitset<16>(serialize_bitset<16>(before).c_str()) ==
              before);
}

TEST(loader_util, raw_to_int) {
  ASSERT_TRUE(raw_to_int<uint16_t>("ab") == 97 + (98 << 8));
}

TEST(loader_util, hhmm_to_int_1) {
  ASSERT_TRUE(hhmm_to_min(parse<int>("0130")) == 90);
}

TEST(loader_util, hhmm_to_int_2) {
  ASSERT_TRUE(hhmm_to_min(parse<int>("")) == 0);
}

TEST(loader_util, hhmm_to_int_3) {
  ASSERT_TRUE(hhmm_to_min(parse<int>("2501")) == 1501);
}

TEST(loader_util, find_nth_1st) {
  auto ints = {1, 2, 3, 4, 5, 4, 3, 2, 1, 2};
  auto it = find_nth(begin(ints), end(ints), 1,
                     std::bind(std::equal_to<>(), 2, std::placeholders::_1));
  ASSERT_TRUE(it != end(ints));
  ASSERT_TRUE(std::distance(begin(ints), it) == 1);
}

TEST(loader_util, find_nth_2nd) {
  auto ints = {1, 2, 3, 4, 5, 4, 3, 2, 1, 2};
  auto it = find_nth(begin(ints), end(ints), 2,
                     std::bind(std::equal_to<>(), 2, std::placeholders::_1));
  ASSERT_TRUE(it != end(ints));
  ASSERT_TRUE(std::distance(begin(ints), it) == 7);
}

TEST(loader_util, find_nth_not_found_contained) {
  auto ints = {1, 2, 3, 4, 5, 4, 3, 2, 1, 2};
  auto it = find_nth(begin(ints), end(ints), 4,
                     std::bind(std::equal_to<>(), 2, std::placeholders::_1));
  ASSERT_TRUE(it == end(ints));
}

TEST(loader_util, find_nth_not_found_not_contained) {
  auto ints = {1, 2, 3, 4, 5, 4, 3, 2, 1, 2};
  auto it = find_nth(begin(ints), end(ints), 1,
                     std::bind(std::equal_to<>(), 7, std::placeholders::_1));
  ASSERT_TRUE(it == end(ints));
}

TEST(loader_util, find_nth_not_found_empty_vec) {
  auto ints = std::vector<int>();
  auto it = find_nth(begin(ints), end(ints), 1,
                     std::bind(std::equal_to<>(), 7, std::placeholders::_1));
  ASSERT_TRUE(it == end(ints));
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
