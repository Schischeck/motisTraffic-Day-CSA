#pragma once

#include "motis/routing/label/comparator.h"
#include "motis/routing/label/criteria/absurdity.h"
#include "motis/routing/label/criteria/late_connections.h"
#include "motis/routing/label/criteria/no_intercity.h"
#include "motis/routing/label/criteria/transfers.h"
#include "motis/routing/label/criteria/travel_time.h"
#include "motis/routing/label/criteria/weighted.h"
#include "motis/routing/label/dominance.h"
#include "motis/routing/label/filter.h"
#include "motis/routing/label/initializer.h"
#include "motis/routing/label/label.h"
#include "motis/routing/label/tie_breakers.h"
#include "motis/routing/label/updater.h"

namespace motis {
namespace routing {

template <search_dir Dir>
using default_label =
    label<Dir, label_data<travel_time, transfers, absurdity>,
          initializer<travel_time_initializer, transfers_initializer,
                      absurdity_initializer>,
          updater<travel_time_updater, transfers_updater, absurdity_updater>,
          filter<travel_time_filter, transfers_filter>,
          dominance<absurdity_tb, travel_time_dominance, transfers_dominance>,
          dominance<absurdity_post_search_tb, travel_time_alpha_dominance,
                    transfers_dominance>,
          comparator<transfers_dominance>,
          get_travel_time_lb, MAX_TRAVEL_TIME>;

template <search_dir Dir>
using default_simple_label = label<
    Dir, label_data<travel_time, transfers>,
    initializer<travel_time_initializer, transfers_initializer>,
    updater<travel_time_updater, transfers_updater>,
    filter<travel_time_filter, transfers_filter>,
    dominance<default_tb, travel_time_dominance, transfers_dominance>,
    dominance<post_search_tb, travel_time_alpha_dominance, transfers_dominance>,
    comparator<transfers_dominance>, get_travel_time_lb,
    MAX_TRAVEL_TIME>;

template <search_dir Dir>
using single_criterion_label =
    label<Dir, label_data<weighted>, initializer<weighted_initializer>,
          updater<weighted_updater>, filter<weighted_filter>,
          dominance<default_tb, weighted_dominance>, dominance<post_search_tb>,
          comparator<weighted_dominance>, get_weighted_lb, MAX_WEIGHTED>;

template <search_dir Dir>
using single_criterion_no_intercity_label =
    label<Dir, label_data<weighted>, initializer<weighted_initializer>,
          updater<weighted_updater>,
          filter<weighted_filter, no_intercity_filter>,
          dominance<default_tb, weighted_dominance>, dominance<post_search_tb>,
          comparator<weighted_dominance>, get_weighted_lb, MAX_WEIGHTED>;

template <search_dir Dir>
using late_connections_label = label<
    Dir, label_data<travel_time, transfers, late_connections, absurdity>,
    initializer<travel_time_initializer, transfers_initializer,
                late_connections_initializer, absurdity_initializer>,
    updater<travel_time_updater, transfers_updater, late_connections_updater,
            absurdity_updater>,
    filter<travel_time_filter, transfers_filter, late_connections_filter>,
    dominance<absurdity_tb, travel_time_dominance, transfers_dominance,
              late_connections_dominance>,
    dominance<absurdity_post_search_tb, travel_time_alpha_dominance,
              transfers_dominance, late_connections_post_search_dominance>,
    comparator<transfers_dominance>, get_travel_time_lb,
    MAX_TRAVEL_TIME>;

template <search_dir Dir>
using late_connections_label_for_tests = label<
    Dir, label_data<travel_time, transfers, late_connections>,
    initializer<travel_time_initializer, transfers_initializer,
                late_connections_initializer>,
    updater<travel_time_updater, transfers_updater, late_connections_updater>,
    filter<travel_time_filter, transfers_filter, late_connections_filter>,
    dominance<default_tb, travel_time_dominance, transfers_dominance,
              late_connections_dominance>,
    dominance<post_search_tb, travel_time_alpha_dominance, transfers_dominance,
              late_connections_post_search_dominance_for_tests>,
    comparator<transfers_dominance>, get_travel_time_lb,
    MAX_TRAVEL_TIME>;

}  // namespace routing
}  // namespace motis
