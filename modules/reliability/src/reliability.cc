#include "motis/reliability/reliability.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/reliability/db_distributions.h"
#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/error.h"

using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace reliability {

po::options_description reliability::desc() {
  po::options_description desc("Reliability Module");
  return desc;
}

void reliability::print(std::ostream&) const {}

reliability::reliability() {}

void reliability::on_msg(msg_ptr msg, sid session_id, callback cb) {
  if (msg->content_type() == MsgContent_ReliableRoutingRequest) {
    auto req = msg->content<ReliableRoutingRequest const*>();

    auto reliability_cb =
        [&](routing::RoutingResponse const* routing_response,
            std::vector<float> ratings, boost::system::error_code e) {
          if (e) {
            return cb({}, e);
          }
          return cb({/* todo: write response */}, error::ok);
        };

    find_connections(req->request(), session_id, reliability_cb);
  } else {
    return cb({}, error::not_implemented);
  }
}

msg_ptr to_flatbuffers_message(routing::RoutingRequest const* request) {
  /* convert routing::RoutingRequest to flatbuffers::Offset<RoutingRequest> */
  flatbuffers::FlatBufferBuilder b;
  std::vector<flatbuffers::Offset<routing::StationPathElement> >
      station_elements;
  for (auto it = request->path()->begin(); it != request->path()->end(); ++it) {
    station_elements.push_back(routing::CreateStationPathElement(
        b, b.CreateString(it->name()->c_str()),
        b.CreateString(it->eva_nr()->c_str())));
  }
  routing::Interval interval(request->interval()->begin(),
                             request->interval()->end());
  b.Finish(CreateMessage(
      b, MsgContent_RoutingRequest,
      routing::CreateRoutingRequest(b, &interval, request->type(),
                                    request->direction(),
                                    b.CreateVector(station_elements)).Union()));
  return make_msg(b);
}

void reliability::find_connections(routing::RoutingRequest const* request,
                                   sid session_id, rating_response_cb cb) {
  return dispatch(
      to_flatbuffers_message(request), session_id,
      std::bind(&reliability::handle_routing_response, this, p::_1, p::_2, cb));
}

void reliability::handle_routing_response(motis::module::msg_ptr msg,
                                          boost::system::error_code e,
                                          rating_response_cb cb) {
  if (e) {
    return cb(nullptr, {}, e);
  }

  auto res = msg->content<routing::RoutingResponse const*>();
  std::vector<float> ratings;
  for (auto it = res->connections()->begin(); it != res->connections()->end();
       ++it) {
    /* todo: rate connection */
    ratings.push_back(0.0);
  }

  return cb(res, ratings, error::ok);
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

  distributions_container::precomputed_distributions_container
      distributions_container(schedule.node_count);
  db_distributions db_distributions(
      "", 120, 120);  // TODO: read max travel time from graph
  distributions_calculator::precomputation::perform_precomputation(
      schedule, db_distributions, distributions_container);

  for (auto const& firstRouteNode : schedule.route_index_to_first_route_node) {
    node const* node = firstRouteNode;
    edge const* edge = nullptr;

    while ((edge = route_edge(node)) != nullptr) {
      auto conInfo = edge->_m._route_edge._conns[0]._full_con->con_info;
      std::cout << "route from "
                << schedule.stations[node->get_station()->_id]->name << " to "
                << schedule.stations[edge->_to->get_station()->_id]->name
                << ": " << schedule.categories[conInfo->family]->name << " "
                << conInfo->train_nr << "\n";

      for (auto const& lightCon : edge->_m._route_edge._conns) {
        std::cout << "  dep=" << lightCon.d_time << " ("
                  << schedule.tracks.at(lightCon._full_con->d_platform) << ")"
                  << ", arr=" << lightCon.d_time << " ("
                  << schedule.tracks.at(lightCon._full_con->a_platform)
                  << ")\n";
      }

      node = edge->_to;
    }
  }
  return true;
}

}  // namespace reliability
}  // namespace motis
