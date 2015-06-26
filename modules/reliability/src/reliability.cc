#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "motis/module/api.h"

using namespace json11;
using namespace td;
using namespace motis::module;

namespace motis {
namespace reliability {

std::vector<Json> get_distribution(reliability* r, Json const& msg) {
  std::cout << "Get Distribution" << std::endl;
  return {Json::object{{"status", "success"}}};
}

reliability::reliability() : ops_{{"get-distribution", get_distribution}} {}

Edge const* route_edge(Node const* routeNode) {
  for (auto const& edge : routeNode->_edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

bool reliability::initialize() {
  for (auto const& firstRouteNode : schedule_->routeIndexToFirstRouteNode) {
    Node const* node = firstRouteNode;
    Edge const* edge = nullptr;

    while ((edge = route_edge(node)) != nullptr) {
      auto conInfo = edge->_m._routeEdge._conns[0]._fullCon->conInfo;
      std::cout << "route from "
                << schedule_->stations[node->getStation()->_id]->name << " to "
                << schedule_->stations[edge->_to->getStation()->_id]->name
                << ": " << schedule_->categoryNames[conInfo->family] << " "
                << conInfo->trainNr << "\n";

      for (auto const& lightCon : edge->_m._routeEdge._conns) {
        std::cout << "  dep="
                  << schedule_->dateManager.formatISO(lightCon.dTime) << " ("
                  << schedule_->tracks[lightCon._fullCon->dPlatform] << ")"
                  << ", arr="
                  << schedule_->dateManager.formatISO(lightCon.aTime) << " ("
                  << schedule_->tracks[lightCon._fullCon->aPlatform] << ")\n";
      }

      node = edge->_to;
    }
  }
  return true;
}

std::vector<Json> reliability::on_msg(Json const& msg, sid) {
  auto op = ops_.find(msg["type"].string_value());
  if (op == end(ops_)) {
    return {};
  }
  return op->second(this, msg);
}

MOTIS_MODULE_DEF_MODULE(reliability)

}  // namespace reliability
}  // namespace motis
