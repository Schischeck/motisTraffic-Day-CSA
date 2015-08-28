#include "motis/loader/graph_builder.h"

namespace motis {
namespace loader {

schedule_ptr build_graph(Schedule const* serialized) {
  schedule_ptr sched(new schedule());

  return sched;
}

}  // loader
}  // motis
