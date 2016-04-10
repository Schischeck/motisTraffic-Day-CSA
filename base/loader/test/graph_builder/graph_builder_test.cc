#include "graph_builder_test.h"

#include <iostream>

#include "motis/loader/graph_builder.h"
#include "motis/loader/hrd/hrd_parser.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

#include "../hrd/test_spec_test.h"

namespace motis {
namespace loader {

loader_graph_builder_test::loader_graph_builder_test(std::string schedule_name,
                                                     std::time_t schedule_begin,
                                                     std::time_t schedule_end)
    : schedule_name_(std::move(schedule_name)),
      schedule_begin_(schedule_begin),
      schedule_end_(schedule_end) {}

void loader_graph_builder_test::SetUp() {
  hrd::hrd_parser parser;

  const auto schedule_path = hrd::SCHEDULES / schedule_name_;
  if (!parser.applicable(schedule_path)) {
    for (auto const& file : parser.missing_files(schedule_path)) {
      std::cout << "- " << file << std::endl;
    }
    FAIL() << "HRD parser not applicable!";
  }

  try {
    flatbuffers::FlatBufferBuilder b;
    parser.parse(schedule_path, b);
    auto serialized = GetSchedule(b.GetBufferPointer());

    sched_ = build_graph(serialized, schedule_begin_, schedule_end_, true, true,
                         false);
  } catch (parser_error const& e) {
    e.print_what();
    FAIL() << "build_graph failed";
  }
}

edge const* loader_graph_builder_test::get_route_edge(node const* route_node) {
  auto it =
      std::find_if(begin(route_node->edges_), end(route_node->edges_),
                   [](edge const& e) { return e.type() == edge::ROUTE_EDGE; });
  if (it == end(route_node->edges_)) {
    return nullptr;
  } else {
    return &(*it);
  }
}

std::vector<std::tuple<light_connection const*, node const*, node const*>>
loader_graph_builder_test::get_connections(node const* first_route_node,
                                           time departure_time) {
  decltype(get_connections(first_route_node, departure_time)) cons;
  edge const* route_edge = nullptr;
  node const* route_node = first_route_node;
  while ((route_edge = get_route_edge(route_node)) != nullptr) {
    auto const* con = route_edge->get_connection(departure_time);
    if (con != nullptr) {
      cons.emplace_back(con, route_node, route_edge->to_);
      route_node = route_edge->to_;
      departure_time = std::get<0>(cons.back())->a_time_;
    } else {
      break;
    }
  }
  return cons;
}

}  // namespace loader
}  // namespace motis
