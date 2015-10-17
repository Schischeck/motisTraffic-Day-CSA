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

  boost::filesystem::path const root = SCHEDULES / "ts_mss_full";

  test_spec s1_spec(root / "fahrten", "services_03040.101");
  test_spec s2_spec(root / "fahrten", "services_03056.101");
  test_spec s3_spec(root / "fahrten", "services_03080.101");
  test_spec s4_spec(root / "fahrten", "services_03775.101");
  test_spec b_spec(root / "stamm", "bitfield.101");
  test_spec ts_spec(root / "stamm", "durchbi.101");
  test_spec ms_spec(root / "stamm", "vereinig_vt.101");

  auto hrd_bitfields = parse_bitfields(b_spec.lf_);

  flatbuffers::FlatBufferBuilder fbb;
  bitfield_translator bt(hrd_bitfields, fbb);

  auto non_expanded_services = {
      s1_spec.get_hrd_services(), s2_spec.get_hrd_services(),
      s3_spec.get_hrd_services(), s4_spec.get_hrd_services()};

  std::vector<hrd_service> expanded_services;
  for (auto const& ss : non_expanded_services) {
    for (auto const& s : ss) {
      expand_traffic_days(s, bt, expanded_services);
    }
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
    printf("remaining services:\n");
    printf("(%s, %d)\n", s.get()->origin_.filename,
           s.get()->origin_.line_number);
    printf("traffic_days: [%s]\n", s.get()->traffic_days_.to_string().c_str());
  }
  for (auto const& rs : service_rs.rule_services_) {
    printf("new service:\n");
    for (auto const& s : rs.rules) {
      printf("(%s, %d)-[%s]-(%s, %d)\n", s.s1->origin_.filename,
             s.s1->origin_.line_number,
             (int)s.rule_info.type == 1 ? "MSSR" : "TSR",
             s.s2->origin_.filename, s.s2->origin_.line_number);
      printf("traffic_days: [%s]\n", s.s1->traffic_days_.to_string().c_str());
      printf("traffic_days: [%s]\n", s.s2->traffic_days_.to_string().c_str());
    }
  }
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
}

}  // loader
}  // motis
}  // hrd
