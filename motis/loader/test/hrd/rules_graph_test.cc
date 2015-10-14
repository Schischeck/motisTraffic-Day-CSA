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

TEST(loader_hrd_rules_graph, through_and_merge_split_services) {
  test_spec s_spec(SCHEDULES / "through-and-merge-split-services" / "fahrten",
                   "services.101");
  test_spec b_spec(SCHEDULES / "through-and-merge-split-services" / "stamm",
                   "bitfield.101");
  test_spec ts_spec(SCHEDULES / "through-and-merge-split-services" / "stamm",
                    "durchbi.101");
  test_spec ms_spec(SCHEDULES / "through-and-merge-split-services" / "stamm",
                    "vereinig_vt.101");

  auto hrd_bitfields = parse_bitfields(b_spec.lf_);

  rules rs;
  parse_through_service_rules(ts_spec.lf_, hrd_bitfields, rs);
  parse_merge_split_service_rules(ms_spec.lf_, hrd_bitfields, rs);
  service_rules service_rs(rs);

  flatbuffers::FlatBufferBuilder fbb;
  bitfield_translator bt(hrd_bitfields, fbb);

  auto non_expanded_services = s_spec.get_hrd_services();

  std::vector<hrd_service> expanded_services;
  for (auto const& s : non_expanded_services) {
    expand_traffic_days(s, bt, expanded_services);
  }
  for (auto const& s : expanded_services) {
    service_rs.add_service(s);
  }

  auto const& layers = service_rs.create_graph();
}

}  // loader
}  // motis
}  // hrd
