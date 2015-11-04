#pragma once

#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {

struct connection_graph_optimizer {
  connection_graph_optimizer(duration const min_departure_diff,
                             duration const interval_width)
      : min_departure_diff_(min_departure_diff),
        interval_width_(interval_width) {}
  virtual ~connection_graph_optimizer() {}
  virtual bool complete(connection_graph::stop const& stop,
                        connection_graph const&) const = 0;

  duration const min_departure_diff_;
  duration const interval_width_;
};

struct simple_optimizer : connection_graph_optimizer {
  simple_optimizer(unsigned int const num_alternatives_at_each_stop,
                   duration const min_departure_diff,
                   duration const interval_width)
      : connection_graph_optimizer(min_departure_diff, interval_width),
        num_alternatives_at_each_stop_(num_alternatives_at_each_stop) {}

  bool complete(connection_graph::stop const& stop,
                connection_graph const&) const override {
    return stop.departure_infos_.size() >= num_alternatives_at_each_stop_;
  }

private:
  unsigned int const num_alternatives_at_each_stop_;

};  // namespace simple_optimizer
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
