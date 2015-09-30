#include <cinttypes>

#include "gtest/gtest.h"

#include "boost/filesystem/path.hpp"

#include "test_spec.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"
#include "motis/loader/parsers/hrd/service/rules_graph.h"

using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_service_rules_graph, prototype) {
  fs::path root(
      "/home/traffel/Workspace/hrd/070_001_RIS_OEV_IMM_J15/rohdaten/stamm");
  test_spec through_services_file(root, "durchbi.101");
  test_spec merge_split_services_file(root, "vereinig_vt.101");

  auto ts_rules = parse_through_service_rules(through_services_file.lf_);
  auto ms_rules = parse_merge_split_service_rules(merge_split_services_file.lf_);
  rules_graph(ts_rules, ms_rules).print_graph();
}

}  // loader
}  // motis
}  // hrd
