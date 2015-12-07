#include "motis/loader/duplicate_checker.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {

duplicate_checker::duplicate_checker(schedule& sched)
    : schedule_(sched), last_service_num_(0), duplicate_count_(0) {}

void duplicate_checker::remove_duplicates() {
  for (auto& station_node_ptr : schedule_.station_nodes) {
    remove_duplicates(station_node_ptr.get());
  }
}

void add_light_conns(edge* e,
                     std::vector<std::pair<time, light_connection*>>& acc,
                     time light_connection::*member) {
  if (e->type() == edge::ROUTE_EDGE) {
    for (auto& lc : e->_m._route_edge._conns) {
      acc.emplace_back(lc.*member, &lc);
    }
  }
}

void duplicate_checker::remove_duplicates(station_node* s) {
  std::vector<std::pair<time, light_connection *>> arr_lcs, dep_lcs;
  for (auto& r : s->get_route_nodes()) {
    for (auto& e : r->_incoming_edges) {
      add_light_conns(e, arr_lcs, &light_connection::a_time);
    }
    for (auto& e : r->_edges) {
      add_light_conns(&e, dep_lcs, &light_connection::d_time);
    }
  }

  std::sort(begin(arr_lcs), end(arr_lcs));
  handle_duplicates(arr_lcs);

  std::sort(begin(dep_lcs), end(dep_lcs));
  handle_duplicates(dep_lcs);
}

void duplicate_checker::handle_duplicates(
    std::vector<std::pair<time, light_connection*>>& candidates) {
  for (unsigned i = 1; i < candidates.size(); ++i) {
    if (is_duplicate_event(candidates.at(i - 1), candidates.at(i))) {
      set_new_service_num(candidates.at(i - 1).second);
    }
  }
}

bool duplicate_checker::is_duplicate_event(
    std::pair<time, light_connection*> const& p1,
    std::pair<time, light_connection*> const& p2) const {
  return p1.first == p2.first &&
         p1.second->_full_con->con_info->train_nr ==
             p2.second->_full_con->con_info->train_nr &&
         p1.second->_full_con->con_info->line_identifier ==
             p2.second->_full_con->con_info->line_identifier;
}

void duplicate_checker::set_new_service_num(light_connection* lc) {
  schedule_.connection_infos.emplace_back(
      new connection_info(*lc->_full_con->con_info));
  auto conn_info_cpy = schedule_.connection_infos.back().get();
  conn_info_cpy->original_train_nr = conn_info_cpy->train_nr;
  conn_info_cpy->train_nr = --last_service_num_;

  schedule_.full_connections.emplace_back(new connection(*lc->_full_con));
  auto full_conn_cpy = schedule_.full_connections.back().get();
  full_conn_cpy->con_info = conn_info_cpy;

  lc->_full_con = full_conn_cpy;
  ++duplicate_count_;
}

unsigned duplicate_checker::get_duplicate_count() { return duplicate_count_; }

}  // namespace loader
}  // namespace motis
