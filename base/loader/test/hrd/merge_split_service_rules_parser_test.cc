#include <cinttypes>

#include "gtest/gtest.h"

#include "motis/schedule-format/RuleService_generated.h"

#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/hrd/parser/merge_split_rules_parser.h"
#include "motis/loader/util.h"

#include "./paths.h"
#include "./test_spec_test.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_ms, multiple_rules) {
  test_spec b_spec(SCHEDULES / "ts-mss-hrd" / "stamm", "bitfield.101");
  auto hrd_bitfields = parse_bitfields(b_spec.lf_, hrd_5_00_8_);
  test_spec ts_spec(SCHEDULES / "ts-mss-hrd" / "stamm", "vereinig_vt.101");
  service_rules rs;
  parse_merge_split_service_rules(ts_spec.lf_, hrd_bitfields, rs, hrd_5_00_8_);

  ASSERT_EQ(3, rs.size());
  auto it_1 = rs.find(std::make_pair(3056, raw_to_int<uint64_t>("07____")));
  ASSERT_TRUE(it_1 != end(rs));

  ASSERT_EQ(2, it_1->second.size());
  auto rule_info = it_1->second[0]->rule_info();
  ASSERT_EQ(8000267, rule_info.eva_num_1_);
  ASSERT_EQ(8000228, rule_info.eva_num_2_);
  auto it_b = hrd_bitfields.find(17524);
  ASSERT_TRUE(it_b != end(hrd_bitfields));
  ASSERT_EQ(it_b->second, rule_info.traffic_days_);
  ASSERT_EQ(RuleType_MERGE_SPLIT, rule_info.type_);

  rule_info = it_1->second[1]->rule_info();
  ASSERT_EQ(8002924, rule_info.eva_num_1_);
  ASSERT_EQ(8003887, rule_info.eva_num_2_);
  it_b = hrd_bitfields.find(37793);
  ASSERT_TRUE(it_b != end(hrd_bitfields));
  ASSERT_EQ(it_b->second, rule_info.traffic_days_);
  ASSERT_EQ(RuleType_MERGE_SPLIT, rule_info.type_);

  auto it_2 = rs.find(std::make_pair(3040, raw_to_int<uint64_t>("07____")));
  ASSERT_TRUE(it_2 != end(rs));
  ASSERT_EQ(1, it_2->second.size());
  ASSERT_EQ(it_1->second[0].get(), it_2->second[0].get());
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
