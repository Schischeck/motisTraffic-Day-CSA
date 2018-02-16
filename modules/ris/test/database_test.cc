#include "gtest/gtest.h"

#include <string>

#include "motis/module/message.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace flatbuffers;
using namespace motis::test;
using namespace motis::test::schedule;
using namespace motis::module;
using motis::test::schedule::simple_realtime::dataset_opt;

namespace motis {
namespace ris {

struct ris_db : public motis_instance_test {
  ris_db() : motis::test::motis_instance_test(dataset_opt, {"ris"}) {}
};

TEST_F(ris_db, no_overlap_before) {}

// TEST_F(ris_db, no_overlap_after) {}
// TEST_F(ris_db, overlap_begin_short) {}
// TEST_F(ris_db, overlap_begin_and_end_short) {}
// TEST_F(ris_db, overlap_end_short) {}
// TEST_F(ris_db, overlap_begin_long) {}
// TEST_F(ris_db, overlap_begin_and_end_long) {}
// TEST_F(ris_db, overlap_end_long) {}
// TEST_F(ris_db, overlap_inside) {}

}  // namespace ris
}  // namespace motis
