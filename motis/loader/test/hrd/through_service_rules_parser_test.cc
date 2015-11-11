#include <cinttypes>

#include "gtest/gtest.h"

#include "test_spec.h"

#include "motis/schedule-format/RuleService_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/hrd/parser/through_services_parser.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_ts, multiple_rules) {
  test_spec b_spec(SCHEDULES / "ts-mss-hrd" / "stamm", "bitfield.101");
  auto hrd_bitfields = parse_bitfields(b_spec.lf_);
  test_spec ts_spec(SCHEDULES / "ts-mss-hrd" / "stamm", "durchbi.101");
  service_rules rs;
  parse_through_service_rules(ts_spec.lf_, hrd_bitfields, rs);

  ASSERT_EQ(4, rs.size());
  auto it_1 = rs.find(
      std::make_pair<int, uint64_t>(3040, raw_to_int<uint64_t>("07____")));
  ASSERT_TRUE(it_1 != end(rs));

  ASSERT_EQ(2, it_1->second.size());
  auto rule_info = it_1->second[0]->rule_info();
  ASSERT_EQ(8000267, rule_info.eva_num_1);
  ASSERT_EQ(8000267, rule_info.eva_num_2);
  auto it_b = hrd_bitfields.find(27351);
  ASSERT_TRUE(it_b != end(hrd_bitfields));
  ASSERT_EQ(it_b->second, rule_info.traffic_days);
  ASSERT_EQ(RuleType_THROUGH, rule_info.type);

  rule_info = it_1->second[1]->rule_info();
  ASSERT_EQ(8000028, rule_info.eva_num_1);
  ASSERT_EQ(8000028, rule_info.eva_num_2);
  it_b = hrd_bitfields.find(0);
  ASSERT_TRUE(it_b != end(hrd_bitfields));
  ASSERT_EQ(it_b->second, rule_info.traffic_days);
  ASSERT_EQ(RuleType_THROUGH, rule_info.type);

  auto it_2 = rs.find(
      std::make_pair<int, uint64_t>(3056, raw_to_int<uint64_t>("07____")));
  ASSERT_TRUE(it_2 != end(rs));
  ASSERT_EQ(2, it_2->second.size());
  ASSERT_EQ(it_1->second[0].get(), it_2->second[0].get());
}

}  // loader
}  // motis
}  // hrd
