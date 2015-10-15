#include <cinttypes>
#include <iostream>

#include "gtest/gtest.h"

#include "test_spec.h"

#include "flatbuffers/flatbuffers.h"

#include "motis/schedule-format/ServiceRules_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/bitfield_translator.h"
#include "motis/loader/parsers/hrd/service/split_service.h"
#include "motis/loader/parsers/hrd/service_rules/service_rules.h"
#include "motis/loader/parsers/hrd/service_rules/through_services_parser.h"
#include "motis/loader/parsers/hrd/service_rules/merge_split_rules_parser.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_rules_graph, ts_mss_full) {

  boost::filesystem::path const root = SCHEDULES / "mss-multiple-once";

  test_spec s_spec(root / "fahrten", "services.101");
  test_spec b_spec(root / "stamm", "bitfield.101");
  test_spec ts_spec(root / "stamm", "durchbi.101");
  test_spec ms_spec(root / "stamm", "vereinig_vt.101");

  auto hrd_bitfields = parse_bitfields(b_spec.lf_);

  flatbuffers::FlatBufferBuilder fbb;
  bitfield_translator bt(hrd_bitfields, fbb);

  auto non_expanded_services = s_spec.get_hrd_services();

  std::vector<hrd_service> expanded_services;
  for (auto const& s : non_expanded_services) {
    expand_traffic_days(s, bt, expanded_services);
  }

  rules rs;
  parse_through_service_rules(ts_spec.lf_, hrd_bitfields, rs);
  parse_merge_split_service_rules(ms_spec.lf_, hrd_bitfields, rs);
  service_rules service_rs(rs);

  for (auto const& s : expanded_services) {
    service_rs.add_service(s);
  }

  service_rs.create_graph();
  for (auto const& s : service_rs.origin_services_) {
    printf("/%p/  traffic_days: [%s]\n", s.get(),
           s.get()->traffic_days_.to_string().c_str());
  }
  for (auto const& seq : service_rs.rule_services_) {
    printf("_new_service_\n");
    for (auto const& rs : seq) {
      printf("/%p/-(%d)->/%p/\n", rs.s1, (int)rs.rule_info.type, rs.s2);
      printf("/%d/ traffic_days: [%s]\n", rs.s1->sections_[0].train_num,
             rs.s1->traffic_days_.to_string().c_str());
      printf("/%d/ traffic_days: [%s]\n", rs.s2->sections_[0].train_num,
             rs.s2->traffic_days_.to_string().c_str());
    }
  }

  ASSERT_EQ(11, service_rs.origin_services_.size());
  ASSERT_EQ(9, service_rs.rule_services_.size());
}

TEST(loader_hrd_rules_graph, ts_twice) {

  boost::filesystem::path const root = SCHEDULES / "ts-twice";

  test_spec s_spec(root / "fahrten", "services.101");
  test_spec b_spec(root / "stamm", "bitfield.101");
  test_spec ts_spec(root / "stamm", "durchbi.101");
  test_spec ms_spec(root / "stamm", "vereinig_vt.101");

  auto hrd_bitfields = parse_bitfields(b_spec.lf_);

  flatbuffers::FlatBufferBuilder fbb;
  bitfield_translator bt(hrd_bitfields, fbb);

  auto non_expanded_services = s_spec.get_hrd_services();

  std::vector<hrd_service> expanded_services;
  for (auto const& s : non_expanded_services) {
    expand_traffic_days(s, bt, expanded_services);
  }

  rules rs;
  parse_through_service_rules(ts_spec.lf_, hrd_bitfields, rs);
  parse_merge_split_service_rules(ms_spec.lf_, hrd_bitfields, rs);
  service_rules service_rs(rs);

  for (auto const& s : expanded_services) {
    service_rs.add_service(s);
  }

  service_rs.create_graph();

  ASSERT_EQ(0, service_rs.origin_services_.size());
  ASSERT_EQ(1, service_rs.rule_services_.size());
  ASSERT_EQ(2, service_rs.rule_services_[0].size());
}

}  // loader
}  // motis
}  // hrd
