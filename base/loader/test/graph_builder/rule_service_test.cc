#include "gtest/gtest.h"

#include <numeric>

#include "motis/core/common/date_util.h"

#include "./graph_builder_test.h"

namespace motis {
namespace loader {

class service_rules_graph_builder_test_real : public loader_graph_builder_test {
public:
  service_rules_graph_builder_test_real()
      : loader_graph_builder_test("cnl-schedule", to_unix_time(2015, 11, 5),
                                  to_unix_time(2015, 11, 7)) {}
};

class service_rules_graph_builder_test_virt : public loader_graph_builder_test {
public:
  service_rules_graph_builder_test_virt()
      : loader_graph_builder_test("mss-ts", to_unix_time(2015, 3, 29),
                                  to_unix_time(2015, 3, 31)) {}

  bool all_stations_exist(std::vector<std::string> const& station_ids) {
    auto const& stations = sched_.get()->stations;
    auto const& station_nodes = sched_.get()->station_nodes;
    return std::all_of(
        begin(station_ids), end(station_ids), [&](std::string const& id) {
          auto s = std::find_if(begin(station_nodes), end(station_nodes),
                                [&](station_node_ptr const& s) {
                                  return stations.at(s.get()->_id)->name == id;
                                });
          if (s == end(station_nodes)) {
            return false;
          } else {
            return true;
          }
        });
  }

  std::vector<edge const*> get_npath(
      std::vector<std::string> const& expected_path) {
    assert(expected_path.size() > 1);

    auto const& stations = sched_.get()->stations;
    auto const& station_nodes = sched_.get()->station_nodes;

    auto const from = std::find_if(begin(station_nodes), end(station_nodes),
                                   [&](station_node_ptr const& s) {
                                     return stations.at(s.get()->_id)->name ==
                                            expected_path.at(0);
                                   });
    if (from == end(station_nodes)) {
      return {};
    }

    std::vector<edge const*> path;
    node const* prev_route_node = nullptr;

    auto augument_path = [&](node const* rn, unsigned i) -> bool {
      for (auto const& re : rn->_edges) {
        if (re.type() == edge::ROUTE_EDGE &&
            stations.at(re._to->_station_node->_id)->name ==
                expected_path.at(i)) {
          path.push_back(&re);
          prev_route_node = path.back()->_to;
          return true;
        }
      }
      return false;
    };

    // find first edge
    for (auto& rn : from->get()->get_route_nodes()) {
      if (augument_path(rn, 1 /* next station*/)) {
        break;
      } else {
        return {};
      }
    }

    // find remaining edges
    for (unsigned i = 2; i < expected_path.size(); ++i) {
      if (augument_path(prev_route_node, i)) {
        continue;
      } else {
        return {};
      }
    }

    return path;
  }
};

}  // loader
}  // motis
