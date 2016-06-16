#include <fstream>
#include <iostream>
#include <vector>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/station.h"
#include "motis/bootstrap/dataset_settings.h"
#include "motis/bootstrap/motis_instance.h"
#include "motis/routes/preprocessing/osm/osm_loader.h"
#include "motis/routes/preprocessing/postgres_writer.h"
#include "motis/routes/preprocessing/station_matcher.h"

using namespace motis;
using namespace motis::bootstrap;
using namespace motis::logging;
using namespace motis::routes;

int main(int argc, char** argv) {
  if (argc < 2 && argc < 4) {
    std::cout << "./routes-preprocessor <osm pbf> <schedule (default "
                 "rohdaten)> <schedule_begin>"
              << std::endl;
    return 0;
  }
  std::string pbf(argv[1]);
  std::string schedule(argc >= 3 ? argv[2] : "rohdaten");
  std::map<int64_t, osm_node> osm_nodes;
  std::map<int64_t, osm_route> osm_routes;
  if (pbf != "/") {
    osm_loader loader(pbf, osm_nodes, osm_routes);
    loader.load_osm();
    LOG(log_level::info) << osm_nodes.size() << " Nodes extracted";
    LOG(log_level::info) << osm_routes.size() << " Routes extracted";
  }
  dataset_settings dataset_opt(schedule, argc == 4 ? argv[3] : "TODAY", 2,
                               false, false, false, false);
  motis_instance instance;
  instance.init_schedule(dataset_opt);
  auto const& sched = *instance.schedule_;
  postgres_writer writer("railways");
  writer.open();
  station_matcher matcher(osm_nodes, osm_routes, sched);
  matcher.find_railways(writer);
  matcher.export_stations(writer);
  writer.close();
}
