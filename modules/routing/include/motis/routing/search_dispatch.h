#pragma once

#include "motis/core/schedule/edges.h"

#include "motis/routing/error.h"
#include "motis/routing/search.h"
#include "motis/routing/start_label_gen.h"

#include "motis/protocol/RoutingRequest_generated.h"

namespace motis {
namespace routing {

template <typename T>
struct get_search_dir {
  typedef T value_type;
};

template <template <search_dir, typename...> class Label, typename... Args>
struct get_search_dir<Label<search_dir::FWD, Args...>> {
  static constexpr auto v = search_dir::FWD;
};

template <template <search_dir, typename...> class Label, typename... Args>
struct get_search_dir<Label<search_dir::BWD, Args...>> {
  static constexpr auto v = search_dir::BWD;
};

template <typename Label, template <search_dir, typename> class Gen>
search_result s(search_query const& q) {
  return search<get_search_dir<Label>::v, Gen<get_search_dir<Label>::v, Label>,
                Label>::get_connections(q);
}

template <search_dir Dir, template <search_dir, typename> class Gen>
search_result search_dispatch(search_query const& q, SearchType const t) {
  switch (t) {
    case SearchType_Default: return s<default_label<Dir>, Gen>(q);
    case SearchType_SingleCriterion:
      return s<single_criterion_label<Dir>, Gen>(q);
    case SearchType_LateConnections:
      return s<late_connections_label<Dir>, Gen>(q);
    case SearchType_LateConnectionsTest:
      return s<late_connections_label_for_tests<Dir>, Gen>(q);
    default: break;
  }
  throw std::system_error(error::search_type_not_supported);
}

search_result search_dispatch(search_query const& q, Start s,
                              SearchType const t, SearchDir d) {
  switch (s) {
    case Start_PretripStart:
      if (d == SearchDir_Forward) {
        return search_dispatch<search_dir::FWD, pretrip_gen>(q, t);
      } else {
        return search_dispatch<search_dir::BWD, pretrip_gen>(q, t);
      }
    case Start_OntripStationStart:
    case Start_OntripTrainStart:
      if (d == SearchDir_Forward) {
        return search_dispatch<search_dir::FWD, ontrip_gen>(q, t);
      } else {
        return search_dispatch<search_dir::BWD, ontrip_gen>(q, t);
      }
      break;
    case Start_NONE: assert(false);
  }
  return {};
}

}  // namespace routing
}  // namespace motis