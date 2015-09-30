#include <cinttypes>

#include "gtest/gtest.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_merge_through_services, basic) {
  char const* file_content =
      "00001 nasTHU 0200351 00002 nasTHU 022664\n"
      "00001 ovfBAY 0462835 00002 ovfBAY 000000\n";

  auto merge_split_rules =
      parse_through_service_rules({"durchbi.101", file_content});

  ASSERT_EQ(2, merge_split_rules.size());

  auto const& first = merge_split_rules[0];
  ASSERT_EQ(1, first.service_key_1.first);
  ASSERT_EQ(raw_to_int<uint64_t>("nasTHU"), first.service_key_1.second);
  ASSERT_EQ(2, first.service_key_2.first);
  ASSERT_EQ(raw_to_int<uint64_t>("nasTHU"), first.service_key_2.second);
  ASSERT_EQ(200351, first.eva_num);
  ASSERT_EQ(22664, first.bitfield_num);

  auto const& second = merge_split_rules[1];
  ASSERT_EQ(1, second.service_key_1.first);
  ASSERT_EQ(raw_to_int<uint64_t>("ovfBAY"), second.service_key_1.second);
  ASSERT_EQ(2, second.service_key_2.first);
  ASSERT_EQ(raw_to_int<uint64_t>("ovfBAY"), second.service_key_2.second);
  ASSERT_EQ(462835, second.eva_num);
  ASSERT_EQ(0, second.bitfield_num);
}

}  // loader
}  // motis
}  // hrd
