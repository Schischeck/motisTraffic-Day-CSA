#pragma once

#include <utility>
#include <vector>
#include <tuple>
#include <memory>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/waiting_time_rules.h"
#include "motis/realtime/delay_info.h"
#include "motis/realtime/waiting_edges.h"
#include "motis/realtime/delay_propagator.h"
#include "motis/realtime/graph_updater.h"
#include "motis/realtime/modified_train.h"
#include "motis/realtime/message_handler.h"
#include "motis/realtime/message_output.h"
#include "motis/realtime/tracking.h"
#include "motis/realtime/statistics.h"

namespace motis {
namespace realtime {

class realtime_schedule {
public:
  realtime_schedule(schedule& schedule);

  std::pair<motis::node*, motis::light_connection*> locate_event(
      const graph_event& event_id) const;
  motis::node* locate_route_node(unsigned station_index,
                                 int32_t route_id) const;

  graph_event get_previous_graph_event(
      const graph_event& ref_graph_event) const;
  schedule_event get_previous_schedule_event(
      const graph_event& ref_graph_event) const;
  graph_event get_next_graph_event(const graph_event& ref_graph_event) const;
  schedule_event get_next_schedule_event(
      const graph_event& ref_graph_event) const;

  schedule_event get_schedule_event(const graph_event& graph_event) const;
  graph_event get_graph_event(const schedule_event& schedule_event) const;

  std::pair<motis::node*, motis::light_connection*> locate_start_of_train(
      motis::node* ref_node, motis::light_connection* ref_lc) const;

  std::tuple<schedule_event, graph_event, motis::node*,
             motis::light_connection*>
  locate_start_of_train(const schedule_event& ref_event) const;

  std::vector<std::tuple<motis::node*, schedule_event, schedule_event>>
  get_train_events(const schedule_event& start_event) const;

  schedule_event find_departure_event(uint32_t train_nr, int day_index) const;

  bool event_exists(const schedule_event& sched_event) const;

  void track_train(uint32_t train_nr);
  bool is_tracked(uint32_t train_nr) const;
  bool is_debug_mode() const { return _debug_mode; }

  bool is_single_train_route(const motis::node* start_node) const;

  motis::node* get_prev_node(motis::node* route_node) const;
  const motis::node* get_prev_node(const motis::node* route_node) const;
  motis::edge* get_prev_edge(motis::node* route_node) const;
  const motis::edge* get_prev_edge(const motis::node* route_node) const;
  motis::edge* get_next_edge(motis::node* route_node) const;
  const motis::edge* get_next_edge(const motis::node* route_node) const;
  motis::node* get_next_node(motis::node* route_node);
  motis::node* get_start_node(motis::node* route_node);

  motis::light_connection* get_connection_with_service(motis::edge* route_edge,
                                                       motis::time start_time,
                                                       uint32_t service) const;
  motis::light_connection* get_connection_with_departure_time(
      motis::edge* route_edge, motis::time departure_time,
      uint32_t train_nr) const;
  motis::light_connection* get_connection_with_arrival_time(
      motis::edge* route_edge, motis::time arrival_time,
      uint32_t train_nr) const;
  motis::light_connection* get_last_connection_with_arrival_before(
      motis::edge* route_edge, motis::time max_time) const;

  motis::schedule& _schedule;
  waiting_edges _waiting_edges;
  delay_info_manager _delay_info_manager;
  delay_propagator _delay_propagator;
  graph_updater _graph_updater;
  modified_train_manager _modified_train_manager;
  message_handler _message_handler;
  message_output _message_output;
  tracking _tracking;
  statistics _stats;

  bool _debug_mode;
  std::vector<uint32_t> _tracked_trains;

  int32_t _max_route_id;
  std::vector<std::unique_ptr<connection>> _new_full_connections;
  std::vector<std::unique_ptr<connection_info>> _new_connection_infos;
};

}  // namespace realtime
}  // namespace motis
