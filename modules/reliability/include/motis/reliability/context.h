#pragma once

namespace motis {
struct schedule;
namespace reliability {
namespace distributions_container {
struct precomputed_distributions_container;
}
struct start_and_travel_distributions;
struct context {
  context(schedule const& schedule,
          distributions_container::precomputed_distributions_container const&
              precomputed_distributions,
          start_and_travel_distributions const& s_t_distributions)
      : schedule_(schedule),
        precomputed_distributions_(precomputed_distributions),
        s_t_distributions_(s_t_distributions) {}
  schedule const& schedule_;
  distributions_container::precomputed_distributions_container const&
      precomputed_distributions_;
  start_and_travel_distributions const& s_t_distributions_;
};
}
}
