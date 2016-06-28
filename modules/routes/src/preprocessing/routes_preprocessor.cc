#include <fstream>
#include <iostream>
#include <vector>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/station.h"

#include "motis/loader/loader.h"

#include "motis/routes/preprocessing/db/railway_finder.h"
#include "motis/routes/preprocessing/db/railway_graph.h"
#include "motis/routes/preprocessing/db/railway_graph_builder.h"
#include "motis/routes/preprocessing/osm/osm_loader.h"
#include "motis/routes/preprocessing/station_matcher.h"

using namespace motis;
using namespace motis::logging;
using namespace motis::routes;
using namespace motis::loader;

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "./routes-preprocessor <geojsons> <schedule (default "
                 "rohdaten)> <schedule_begin>"
              << std::endl;
    return 0;
  }

  loader_options opt(argv[2], argv[3], 2, false, false, false, false);
  auto schedule = load_schedule(opt);

  railway_graph graph;
  railway_graph_builder graph_builder(graph);
  graph_builder.build_graph(argv[1]);

  // railway_finder finder(sched, graph);
}
