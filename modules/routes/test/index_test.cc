#include "gtest/gtest.h"

#include "boost/filesystem.hpp"

#include "motis/module/message.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

#include "motis/routes/db/db_builder.h"
#include "motis/routes/db/mapdb.h"
#include "motis/routes/lookup_index.h"
#include "motis/routes/prepare/prepare_data.h"
#include "motis/routes/routes.h"

#include "motis/routes/prepare/bus_stop_positions.h"
#include "motis/routes/prepare/station_sequences.h"
#include "parser/file.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
namespace fs = boost::filesystem;
using motis::test::schedule::simple_realtime::dataset_opt;

namespace motis {
namespace routes {

constexpr auto kIndexZeroRequest = R""(
{ "destination": {"type": "Module", "target": "/routes/index"},
  "content_type": "RoutesIndexRequest",
  "content": {
    "index": 0
  }
}
)"";

struct routes_index_test : public motis_instance_test {
  routes_index_test()
      : motis_instance_test(dataset_opt, {"routes"}, {"--routes.db=mem:db"}) {
    setup();
  }

  void setup() {
    db_builder b(*get_routes_module().db_);

    b.append({"0", "1"}, {0, 1}, {});
    b.append({"0", "2"}, {0}, {});
    b.append({"0", "3", "4", "5"}, {2, 1, 3}, {});

    b.finish();
    auto buf = get_routes_module().db_->get("__index");
    get_routes_module().lookup_ = std::make_unique<lookup_index>(buf);
  }

  routes& get_routes_module() { return get_module<routes>("routes"); }
};

TEST_F(routes_index_test, index) {
  auto msg = call(make_msg(kIndexZeroRequest));
  auto resp = motis_content(RoutesSeqResponse, msg);
  std::vector<std::string> expected_ids = {"0", "1"};
  std::vector<uint32_t> expected_classes = {0, 1};

  int index = 0;
  for (auto s : *resp->station_ids()) {
    ASSERT_EQ(expected_ids[index], s->str());
    index++;
  }
  index = 0;
  for (auto c : *resp->classes()) {
    ASSERT_EQ(expected_classes[index], c);
    index++;
  }
}

TEST_F(routes_index_test, pipeline) {
  map_database db("");
  auto schedule_file = fs::path("./test/schedule/simple_realtime/schedule.raw");
  if (!fs::is_regular_file(schedule_file)) {
    std::cerr << "cannot open schedule.raw\n";
  }

  auto const schedule_buf =
      parser::file(schedule_file.string().c_str(), "r").content();
  auto const schedule = loader::GetSchedule(schedule_buf.buf_);
  auto const stop_positions = find_bus_stop_positions(
      schedule, "/home/jonas/Data/darmstadt_germany.osm.pbf");
  auto sequences = load_station_sequences(schedule);
  prepare(sequences, stop_positions,
          "/home/jonas/Data/darmstadt_germany.osm.pbf", db);
}

}  // namespace routes
}  // namespace motis