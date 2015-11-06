#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <ostream>

#include "boost/functional/hash.hpp"

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/waiting_time_rules.h"
#include "motis/core/schedule/schedule.h"
#include "motis/realtime/event.h"

namespace motis {
namespace realtime {

class waiting_edge;
class realtime_schedule;

class single_waiting_edge {
public:
  single_waiting_edge(const schedule_event& feeder_arrival,
                      const schedule_event& connector_departure,
                      int waiting_time, waiting_edge* full_waiting_edge)
      : _feeder_arrival(feeder_arrival),
        _connector_departure(connector_departure),
        _waiting_time(waiting_time),
        _full_waiting_edge(full_waiting_edge) {}

  const schedule_event _feeder_arrival;
  const schedule_event _connector_departure;
  int _waiting_time;
  waiting_edge* _full_waiting_edge;
};

class lc_pair {
public:
  lc_pair(motis::time feeder_arrival_time, motis::time connector_departure_time,
          uint32_t feeder_train_nr, uint32_t connector_train_nr)
      : _feeder_a_time(feeder_arrival_time),
        _connector_d_time(connector_departure_time),
        _feeder_train_nr(feeder_train_nr),
        _connector_train_nr(connector_train_nr) {}

  explicit lc_pair(const single_waiting_edge& swe)
      : _feeder_a_time(swe._feeder_arrival._schedule_time),
        _connector_d_time(swe._connector_departure._schedule_time),
        _feeder_train_nr(swe._feeder_arrival._train_nr),
        _connector_train_nr(swe._connector_departure._train_nr) {}

  bool operator==(const lc_pair& other) const {
    return _feeder_a_time == other._feeder_a_time &&
           _connector_d_time == other._connector_d_time &&
           _feeder_train_nr == other._feeder_train_nr &&
           _connector_train_nr == other._connector_train_nr;
  }

  motis::time _feeder_a_time;
  motis::time _connector_d_time;
  uint32_t _feeder_train_nr;
  uint32_t _connector_train_nr;
};

class waiting_edge {
public:
  waiting_edge(unsigned station_index, int32_t from_route_id,
               int32_t to_route_id, int waiting_time)
      : _station_index(station_index),
        _from_route_id(from_route_id),
        _to_route_id(to_route_id),
        _waiting_time(waiting_time) {}

  void add_feeder_arrival_events(motis::time connector_departure_time,
                                 std::vector<single_waiting_edge>& events);
  void add_connector_departure_events(motis::time feeder_arrival_time,
                                      std::vector<single_waiting_edge>& events);

  const unsigned _station_index;
  const int32_t _from_route_id;
  const int32_t _to_route_id;
  const int _waiting_time;
  std::vector<lc_pair> _lc_pairs;
};

class waiting_edges {
public:
  waiting_edges(realtime_schedule& rts, motis::waiting_time_rules& wtr)
      : _rts(rts), _wtr(wtr) {}
  ~waiting_edges();

  void create_waiting_edges();

  const std::vector<single_waiting_edge> get_edges_from(
      const schedule_event& feeder_arrival, int32_t route_id) const;
  const std::vector<single_waiting_edge> get_edges_to(
      const schedule_event& connector_departure, int32_t route_id) const;

  void event_moved_to_new_route(const schedule_event& event,
                                int32_t old_route_id, int32_t new_route_id);

  void add_additional_edge(const schedule_event& feeder_arrival,
                           const schedule_event& connector_departure,
                           int waiting_time);
  void remove_additional_edge(const schedule_event& feeder_arrival,
                              const schedule_event& connector_departure);

  // for debugging
  std::vector<uint32_t> get_incoming_dependencies(uint32_t train_nr) const;
  void write_dependency_graph(std::ostream& out) const;

  realtime_schedule& _rts;
  motis::waiting_time_rules& _wtr;

  using index_type = std::pair<uint32_t, int32_t>;  // station_index, route_id

private:
  std::unordered_map<index_type, std::vector<waiting_edge*>,
                     boost::hash<index_type>> _outgoing_edges;
  std::unordered_map<index_type, std::vector<waiting_edge*>,
                     boost::hash<index_type>> _incoming_edges;

  std::unordered_map<schedule_event, std::vector<single_waiting_edge*>,
                     boost::hash<schedule_event>> _additional_outgoing_edges;
  std::unordered_map<schedule_event, std::vector<single_waiting_edge*>,
                     boost::hash<schedule_event>> _additional_incoming_edges;

  void create_waiting_edges(const motis::edge* feeder,
                            const motis::edge* connector,
                            const motis::light_connection* first_feeder_lc,
                            int feeder_category, int station_transfer_time);

  void store_waiting_edge(waiting_edge* we);

  void move_to_new_route(const single_waiting_edge& swe,
                         int32_t new_feeder_route_id,
                         int32_t new_connector_route_id);

  // DEBUGGING
  void log_light_connection(const motis::light_connection* lc);
};

}  // namespace realtime
}  // namespace motis
