#include "motis/loader/duplicate_checker.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {

duplicate_checker::duplicate_checker(schedule& sched)
    : schedule_(sched), last_service_num_(0), duplicate_count_(0) {}

void duplicate_checker::remove_duplicates() {
  for (auto& station_node_ptr : schedule_.station_nodes_) {
    remove_duplicates(station_node_ptr.get());
  }
}

void add_light_conns(edge* e,
                     std::vector<std::pair<time, light_connection*>>& acc,
                     time light_connection::*member) {
  if (e->type() == edge::ROUTE_EDGE) {
    for (auto& lc : e->m_.route_edge_.conns_) {
      acc.emplace_back(lc.*member, &lc);
    }
  }
}

void duplicate_checker::remove_duplicates(station_node* s) {
  std::vector<std::pair<time, light_connection*>> arr_lcs, dep_lcs;
  for (auto& r : s->get_route_nodes()) {
    for (auto& e : r->incoming_edges_) {
      add_light_conns(e, arr_lcs, &light_connection::a_time_);
    }
    for (auto& e : r->edges_) {
      add_light_conns(&e, dep_lcs, &light_connection::d_time_);
    }
  }

  std::sort(begin(arr_lcs), end(arr_lcs));
  handle_duplicates(arr_lcs);

  std::sort(begin(dep_lcs), end(dep_lcs));
  handle_duplicates(dep_lcs);
}

void duplicate_checker::handle_duplicates(
    std::vector<std::pair<time, light_connection*>>& candidates) {
  for (auto it = begin(candidates); it != end(candidates); ++it) {
    for (auto check_it = std::next(it); check_it != end(candidates);
         ++check_it) {
      if (it->first != check_it->first) {
        break;
      }

      if (is_duplicate_event(*it, *check_it)) {
        set_new_service_num(check_it->second);
      }
    }
  }
}

bool duplicate_checker::is_duplicate_event(
    std::pair<time, light_connection*> const& p1,
    std::pair<time, light_connection*> const& p2) const {
  return p1.first == p2.first &&
         p1.second->full_con_->con_info_->train_nr_ ==
             p2.second->full_con_->con_info_->train_nr_ &&
         p1.second->full_con_->con_info_->line_identifier_ ==
             p2.second->full_con_->con_info_->line_identifier_;
}

void duplicate_checker::set_new_service_num(light_connection* lc) {
  auto conn_info_cpy = schedule_.connection_infos_
                           .emplace_back(std::make_unique<connection_info>(
                               *lc->full_con_->con_info_))
                           .get();
  conn_info_cpy->original_train_nr_ = conn_info_cpy->train_nr_;
  conn_info_cpy->train_nr_ = --last_service_num_;

  auto full_conn_cpy =
      schedule_.full_connections_
          .emplace_back(std::make_unique<connection>(*lc->full_con_))
          .get();
  full_conn_cpy->con_info_ = conn_info_cpy;

  lc->full_con_ = full_conn_cpy;
  ++duplicate_count_;
}

unsigned duplicate_checker::get_duplicate_count() { return duplicate_count_; }

}  // namespace loader
}  // namespace motis
