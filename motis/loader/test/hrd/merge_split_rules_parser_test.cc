#include <cinttypes>

#include "gtest/gtest.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_merge_split_rules, basic) {
  char const* file_content =
      "5400202  8100003  00407  51____  60477  NZ____ 016847\n"
      "5500003  8100002  00462  55____  60466  55____ 000000\n";

  auto merge_split_rules =
      parse_merge_split_rules({"vereinig_vt.101", file_content});

  ASSERT_EQ(2, merge_split_rules.size());

  auto const& first = merge_split_rules[0];
  ASSERT_EQ(407, first.service_key_1.first);
  ASSERT_EQ(raw_to_int<uint64_t>("51____"), first.service_key_1.second);
  ASSERT_EQ(60477, first.service_key_2.first);
  ASSERT_EQ(raw_to_int<uint64_t>("NZ____"), first.service_key_2.second);
  ASSERT_EQ(5400202, first.eva_num_begin);
  ASSERT_EQ(8100003, first.eva_num_end);
  ASSERT_EQ(16847, first.bitfield_num);

  auto const& second = merge_split_rules[1];
  ASSERT_EQ(462, second.service_key_1.first);
  ASSERT_EQ(raw_to_int<uint64_t>("55____"), second.service_key_1.second);
  ASSERT_EQ(60466, second.service_key_2.first);
  ASSERT_EQ(raw_to_int<uint64_t>("55____"), second.service_key_2.second);
  ASSERT_EQ(5500003, second.eva_num_begin);
  ASSERT_EQ(8100002, second.eva_num_end);
  ASSERT_EQ(0, second.bitfield_num);
}

}  // loader
}  // motis
}  // hrd
