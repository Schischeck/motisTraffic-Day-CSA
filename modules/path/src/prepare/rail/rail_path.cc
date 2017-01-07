#include "motis/path/prepare/rail/rail_path.h"

#include "parser/util.h"

#include "utl/concat.h"

namespace motis {
namespace path {

double length(rail_graph const& graph, rail_path const& path) {
  verify(path.valid_, "trying to get length of invalid path");

  if (path.source_->info_idx_ == path.goal_->info_idx_) {
    return std::abs(static_cast<double>(path.source_->dist_) -
                    static_cast<double>(path.goal_->dist_));
  }

  auto const& source_info = graph.infos_[path.source_->info_idx_];
  auto const& source_from_idx = source_info->from_idx();
  auto const& source_to_idx = source_info->to_idx();

  auto const& source_from_dist = path.source_->dist_;
  auto const& source_to_dist = source_info->dist_ - path.source_->dist_;

  auto const& goal_info = graph.infos_[path.goal_->info_idx_];
  auto const& goal_from_idx = goal_info->from_idx();
  auto const& goal_to_idx = goal_info->to_idx();

  auto const& goal_from_dist = path.goal_->dist_;
  auto const& goal_to_dist = goal_info->dist_ - path.goal_->dist_;

  // source --> X --> goal
  if (source_to_idx == goal_from_idx) {

    std::cout << source_from_idx << " " << source_to_idx << std::endl;
    std::cout << goal_from_idx << " " << goal_to_idx << std::endl;
    std::cout << path.edges_.size() << std::endl;
    for(auto const& edge : path.edges_) {
      std::cout << edge->from_->idx_ << " " << edge->from_->idx_ << std::endl;
      std::cout << edge->from_->id_ << " " << edge->from_->id_ << std::endl;

      std::cout << graph.infos_[edge->info_idx_]->polyline_.size() << std::endl;
    }

    verify(path.edges_.empty(), "invalid path!? (fw/len)");
    return source_to_dist + goal_from_dist;

    // source <-- X <-- goal
  } else if (source_from_idx == goal_to_idx) {
    verify(path.edges_.empty(), "invalid path!? (bw/len)");
    return source_from_dist + goal_to_dist;

  } else {
    verify(!path.edges_.empty(), "rail path empty unexpected");
    auto const& front_edge = path.edges_.front();
    auto const& front_from_idx =
        graph.infos_[front_edge->info_idx_]->from_idx(front_edge->is_forward());
    auto const source_dist =
        (source_from_idx == front_from_idx) ? source_from_dist : source_to_dist;

    auto edges_dist = 0.;
    for (auto const& edge : path.edges_) {
      verify(edge != nullptr, "rail path contains nullptr edge");
      edges_dist += edge->dist_;
    }

    auto const& back_edge = path.edges_.back();
    auto const& back_to_idx =
        graph.infos_[back_edge->info_idx_]->to_idx(back_edge->is_forward());
    auto const goal_dist =
        (goal_from_idx == back_to_idx) ? goal_from_dist : goal_to_dist;

    return source_dist + edges_dist + goal_dist;
  }
}

void slice(geo::polyline const& orig, geo::polyline& result,  //
           int const from, int const to) {
  int const inc = (from < to) ? 1 : -1;
  for (int i = from; i != (to + inc); i += inc) {
    result.push_back(orig[i]);
  }
}

geo::polyline to_polyline(rail_graph const& graph, rail_path const& path) {
  verify(path.valid_, "trying to get polyline of invalid path");

  auto const& source_info = graph.infos_[path.source_->info_idx_];
  auto const& source_from_idx = source_info->from_idx();
  auto const& source_to_idx = source_info->to_idx();

  auto const& source_pt = path.source_->point_idx_;
  auto const& source_poly = source_info->polyline_;
  auto const& source_zero = 0;
  auto const& source_last = source_info->polyline_.size() - 1;

  auto const& goal_info = graph.infos_[path.goal_->info_idx_];
  auto const& goal_from_idx = goal_info->from_idx();
  auto const& goal_to_idx = goal_info->to_idx();

  auto const& goal_pt = path.goal_->point_idx_;
  auto const& goal_poly = goal_info->polyline_;
  auto const& goal_zero = 0;
  auto const& goal_last = goal_info->polyline_.size() - 1;

  if (path.source_->info_idx_ == path.goal_->info_idx_) {
    geo::polyline result;
    slice(source_poly, result, source_pt, goal_pt);
    return result;
  }

  // source --> X --> goal
  if (source_to_idx == goal_from_idx) {
    verify(path.edges_.empty(), "invalid path!? (fw/poly)");
    geo::polyline result;
    slice(source_poly, result, source_pt, source_last);
    slice(goal_poly, result, goal_zero, goal_pt);
    return result;

    // source <-- X <-- goal
  } else if (source_from_idx == goal_to_idx) {
    verify(path.edges_.empty(), "invalid path!? (bw/poly)");
    geo::polyline result;
    slice(source_poly, result, source_pt, source_zero);
    slice(goal_poly, result, goal_last, goal_pt);
    return result;
  } else {
    verify(!path.edges_.empty(), "rail path empty unexpected");
    geo::polyline result;

    auto const& front_edge = path.edges_.front();
    auto const& front_from_idx =
        graph.infos_[front_edge->info_idx_]->from_idx(front_edge->is_forward());
    if (source_from_idx == front_from_idx) {
      slice(source_poly, result, source_pt, source_zero);
    } else {
      slice(source_poly, result, source_pt, source_last);
    }

    for (auto const& edge : path.edges_) {
      verify(edge != nullptr, "rail path contains nullptr edge");
      auto const& polyline = graph.infos_[edge->info_idx_]->polyline_;

      auto const e_zero = 0;
      auto const e_last = polyline.size() - 1;

      if (edge->is_forward()) {
        slice(polyline, result, e_zero, e_last);
      } else {
        slice(polyline, result, e_last, e_zero);
      }
    }

    auto const& back_edge = path.edges_.back();
    auto const& back_to_idx =
        graph.infos_[back_edge->info_idx_]->to_idx(back_edge->is_forward());
    if (goal_from_idx == back_to_idx) {
      slice(goal_poly, result, goal_zero, goal_pt);
    } else {
      slice(goal_poly, result, goal_last, goal_pt);
    }

    return result;
  }
}

}  // namespace path
}  // namespace motis
