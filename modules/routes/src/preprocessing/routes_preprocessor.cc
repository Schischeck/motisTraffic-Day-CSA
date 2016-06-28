#include <fstream>
#include <iostream>
#include <vector>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/station.h"
#include "motis/bootstrap/dataset_settings.h"
#include "motis/bootstrap/motis_instance.h"
#include "motis/routes/preprocessing/db/railway_finder.h"
#include "motis/routes/preprocessing/db/railway_graph.h"
#include "motis/routes/preprocessing/db/railway_graph_builder.h"
#include "motis/routes/preprocessing/db/railway_node.h"
#include "motis/routes/preprocessing/osm/osm_loader.h"
#include "motis/routes/preprocessing/station_matcher.h"

using namespace motis;
using namespace motis::bootstrap;
using namespace motis::logging;
using namespace motis::routes;

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "./routes-preprocessor <geojsons> <schedule (default "
                 "rohdaten)> <schedule_begin>"
              << std::endl;
    return 0;
  }
  std::string root(argv[1]);
  std::string schedule(argv[2]);
  dataset_settings dataset_opt(schedule, argv[3], 2, false, false, false,
                               false);
  motis_instance instance;
  instance.init_schedule(dataset_opt);
  auto const& sched = *instance.schedule_;
  railway_graph graph;
  railway_graph_builder graph_builder(graph);
  graph_builder.build_graph(root);

  railway_finder finder(sched, graph);
}
