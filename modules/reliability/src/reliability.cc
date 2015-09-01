#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

#include "motis/reliability/db_distributions.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/error.h"
#include "motis/reliability/train_distributions_calculator.h"

using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace reliability {

po::options_description reliability::desc() {
  po::options_description desc("Reliability Module");
  return desc;
}

void reliability::print(std::ostream&) const {}

reliability::reliability() {}

void reliability::on_msg(msg_ptr msg, sid, callback cb) {
  return cb({}, error::not_implemented);
}

edge const* route_edge(node const* route_node) {
  for (auto const& edge : route_node->_edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

bool reliability::initialize() {
  auto const lock = synced_sched<RO>();
  schedule const& schedule = lock.sched();

  train_distributions_container distributions_container(schedule.node_count);
  db_distributions db_distributions(
      "", 120,
      120);  // TODO: read max travel time from graph
  train_distributions_calculator calculator(schedule, distributions_container,
                                            db_distributions);

  for (auto const& firstRouteNode : schedule.route_index_to_first_route_node) {
    node const* node = firstRouteNode;
    edge const* edge = nullptr;

    while ((edge = route_edge(node)) != nullptr) {
      auto conInfo = edge->_m._route_edge._conns[0]._full_con->con_info;
      std::cout << "route from "
                << schedule.stations[node->get_station()->_id]->name << " to "
                << schedule.stations[edge->_to->get_station()->_id]->name
                << ": " << schedule.category_names[conInfo->family] << " "
                << conInfo->train_nr << "\n";

      for (auto const& lightCon : edge->_m._route_edge._conns) {
        std::cout << "  dep=" << schedule.date_mgr.format_ISO(lightCon.d_time)
                  << " (" << schedule.tracks.at(lightCon._full_con->d_platform)
                  << ")"
                  << ", arr=" << schedule.date_mgr.format_ISO(lightCon.d_time)
                  << " (" << schedule.tracks.at(lightCon._full_con->a_platform)
                  << ")\n";
      }

      node = edge->_to;
    }
  }
  return true;
}

MOTIS_MODULE_DEF_MODULE(reliability)

}  // namespace reliability
}  // namespace motis
