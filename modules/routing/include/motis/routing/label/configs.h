#pragma once

#include "motis/routing/label/comparator.h"
#include "motis/routing/label/criteria/transfers.h"
#include "motis/routing/label/criteria/travel_time.h"
#include "motis/routing/label/dominance.h"
#include "motis/routing/label/filter.h"
#include "motis/routing/label/initializer.h"
#include "motis/routing/label/label.h"
#include "motis/routing/label/tie_breakers.h"
#include "motis/routing/label/updater.h"

namespace motis {
namespace routing {

typedef label<
    label_data<travel_time, transfers>,
    initializer<travel_time_initializer, transfers_initializer>,
    updater<travel_time_updater, transfers_updater>,
    filter<travel_time_filter, transfers_filter>,
    dominance<default_tb, travel_time_dominance, transfers_dominance>,
    dominance<post_search_tb, travel_time_alpha_dominance, transfers_dominance>,
    comparator<travel_time_dominance, transfers_dominance>>
    default_label;

typedef label<label_data<travel_time>, initializer<travel_time_initializer>,
              updater<travel_time_updater>, filter<travel_time_filter>,
              dominance<default_tb, travel_time_dominance>,
              dominance<post_search_tb, travel_time_alpha_dominance>,
              comparator<travel_time_dominance>>
    single_criterion_label;

}  // namespace routing
}  // namespace motis
