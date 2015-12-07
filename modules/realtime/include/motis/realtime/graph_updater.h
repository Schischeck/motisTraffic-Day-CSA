#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <iostream>

#include "motis/core/schedule/delay_info.h"
#include "motis/realtime/modified_train.h"
#include "motis/realtime/delay_info_manager.h"

namespace motis {
namespace realtime {

class realtime_schedule;

struct graph_train_edge {
  graph_train_edge(motis::node* from_route_node, motis::light_connection* lc,
                   delay_info_update dep_update, delay_info_update arr_update)
      : _from_route_node(from_route_node),
        _lc(lc),
        _dep_update(dep_update),
        _arr_update(arr_update) {}

  motis::time new_departure_time() const {
    return _dep_update.valid() ? _dep_update._new_time : _lc->d_time;
  }

  motis::time new_arrival_time() const {
    return _arr_update.valid() ? _arr_update._new_time : _lc->a_time;
  }

  friend std::ostream& operator<<(std::ostream& os, const graph_train_edge& e) {
    os << "<gte from={route_id=" << e._from_route_node->_route << ", "
       << "station=" << e._from_route_node->get_station()->_id << "}, lc={"
       << motis::format_time(e._lc->d_time);
    if (e._dep_update.valid()) {
      os << " [new: " << motis::format_time(e._dep_update._new_time) << " "
         << e._dep_update._new_reason << "]";
    }
    os << " -> " << motis::format_time(e._lc->a_time);
    if (e._arr_update.valid()) {
      os << " [new: " << motis::format_time(e._arr_update._new_time) << " "
         << e._arr_update._new_reason << "]";
    }
    os << ", train_nr=" << e._lc->_full_con->con_info->train_nr << ">";
    return os;
  }

  motis::node* _from_route_node;
  motis::light_connection* _lc;
  delay_info_update _dep_update;
  delay_info_update _arr_update;
};

struct graph_train_info {
  graph_train_info() : _extract_required(false), _times_valid(true) {}

  friend std::ostream& operator<<(std::ostream& os,
                                  const graph_train_info& gti) {
    os << "graph_train_info:"
       << "\n  - extract_required: " << gti._extract_required
       << "\n  - times_valid: " << gti._times_valid << "\n  - edges:";
    for (const auto& e : gti._edges) {
      os << "\n    " << e;
    }
    os << std::endl;
    return os;
  }

  std::vector<graph_train_edge> _edges;
  bool _extract_required;
  bool _times_valid;
};

class graph_updater {
public:
  graph_updater(realtime_schedule& rts) : _rts(rts) {}

  void perform_updates(std::vector<delay_info_update>& updates);

  std::pair<
      std::vector<std::tuple<motis::node*, schedule_event, schedule_event>>,
      modified_train*>
  get_current_events_and_make_modified_train(schedule_event ref_event,
                                             bool get_current_events = true);
  modified_train* make_modified_train(motis::node* start_route_node,
                                      motis::light_connection* start_lc);
  void adjust_train(modified_train* mt, std::vector<schedule_event> new_events,
                    bool update_all_edges = false);

  void finish_graph_update();

private:
  void perform_update(const delay_info_update& update,
                      std::vector<delay_info_update>& other_updates);
  void update_train_times(std::vector<delay_info_update>& updates,
                          motis::node* route_node, motis::light_connection* lc);
  graph_train_info get_graph_train_info(
      motis::node* start_node, motis::light_connection* start_lc,
      std::vector<delay_info_update>& updates);
  graph_train_info extract_route(const graph_train_info& gti);

  motis::node* copy_route_node(motis::node* original_node,
                               uint32_t new_route_id);
  motis::node* create_route_node(unsigned station_index, int32_t route_id,
                                 bool enter, bool leave);
  void delete_route_node(motis::node* route_node);
  void fix_foot_edges(motis::node* route_node, bool enter, bool leave);
  void remove_incoming_edges(motis::node* node);
  void add_incoming_edges(motis::node* node);

  inline bool is_same_train(const motis::light_connection* lc1,
                            const motis::light_connection* lc2) const {
    return lc1->_full_con->con_info->train_nr ==
           lc2->_full_con->con_info->train_nr;
  }

  bool has_entering_edge(const motis::node* route_node) const;
  bool has_leaving_edge(const motis::node* route_node) const;

public:  // temp
  bool check_route(motis::node* ref_node, bool require_times = false) const;
  void dump_route(motis::node* start_node, const char* title,
                  int day_index = -1) const;

private:
  bool check_updates(std::vector<delay_info_update>& updates) const;

  realtime_schedule& _rts;
};

}  // namespace realtime
}  // namespace motis
