#pragma once

#include <ctime>
#include <vector>
#include <map>

#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"
#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/timezone.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/connection.h"

#include "motis/loader/bitfield.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace loader {

struct route_info {
  route_info() : route_node(nullptr), outgoing_route_edge_index(-1) {}
  route_info(node* rn, int edge_idx)
      : route_node(rn), outgoing_route_edge_index(edge_idx) {
    assert(rn == nullptr || rn->is_route_node());
  }

  edge* get_route_edge() {
    if (outgoing_route_edge_index == -1) {
      return nullptr;
    }

    assert(outgoing_route_edge_index >= 0);
    assert(static_cast<unsigned>(outgoing_route_edge_index) <
           route_node->_edges.size());
    assert(route_node->_edges[outgoing_route_edge_index].type() ==
           edge::ROUTE_EDGE);
    return &route_node->_edges[outgoing_route_edge_index];
  }

  node* route_node;
  int outgoing_route_edge_index;
};

typedef std::vector<route_info> route;

struct graph_builder {
  graph_builder(schedule& sched, Interval const* schedule_interval, time_t from,
                time_t to, bool apply_rules);

  void add_stations(
      flatbuffers::Vector<flatbuffers::Offset<Station>> const* stations);

  timezone const* get_or_create_timezone(Timezone const* input_timez);

  void add_services(
      flatbuffers::Vector<flatbuffers::Offset<Service>> const* services);

  void index_first_route_node(route const& r);

  void add_route_services(std::vector<Service const*> const& services);

  static int get_index(
      std::vector<std::vector<light_connection>> const& alt_route,
      std::vector<light_connection> const& sections);

  void add_to_route(std::vector<std::vector<light_connection>>& route,
                    std::vector<light_connection> const& sections, int index);

  void add_to_routes(
      std::vector<std::vector<std::vector<light_connection>>>& alt_routes,
      std::vector<light_connection> const& sections);

  light_connection section_to_connection(Service const* s, int day,
                                         unsigned section_idx, time prev_arr,
                                         bool& adjusted);

  void add_footpaths(
      flatbuffers::Vector<flatbuffers::Offset<Footpath>> const* footpaths);

  void connect_reverse();

  void sort_connections();

  int node_count() const;

  bitfield const& get_or_create_bitfield(
      flatbuffers::String const* serialized_bitfield);

  void read_attributes(
      int day,
      flatbuffers::Vector<flatbuffers::Offset<Attribute>> const* attributes,
      std::vector<attribute const*>& active_attributes);

  std::string const* get_or_create_direction(Direction const* dir);

  provider const* get_or_create_provider(Provider const* p);

  int get_or_create_category_index(Category const* c);

  int get_or_create_platform(
      int day,
      flatbuffers::Vector<flatbuffers::Offset<Platform>> const* platforms);

  std::unique_ptr<route> create_route(Route const* r, unsigned route_index);

  route_info add_route_section(int route_index, Station const* from_stop,
                               bool in_allowed, bool out_allowed,
                               edge* last_route_edge,
                               bool build_outgoing_route_edge,
                               node* route_node = nullptr);

  unsigned duplicate_count_;
  unsigned next_route_index_;
  std::map<Category const*, int> categories_;
  std::map<std::string, int> tracks_;
  std::map<AttributeInfo const*, attribute*> attributes_;
  std::map<flatbuffers::String const*, std::string const*> directions_;
  std::map<Provider const*, provider const*> providers_;
  hash_map<Station const*, station_node*> stations_;
  std::map<Timezone const*, timezone const*> timezones_;
  hash_map<flatbuffers::String const*, bitfield> bitfields_;
  hash_set<connection_info*,
           deep_ptr_hash<connection_info::hash, connection_info>,
           deep_ptr_eq<connection_info>> con_infos_;
  hash_set<connection*, deep_ptr_hash<connection::hash, connection>,
           deep_ptr_eq<connection>> connections_;
  unsigned next_node_id_;
  schedule& sched_;
  int first_day_, last_day_;
  bool apply_rules_;

  connection_info con_info_;
  connection con_;
};

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool check_unique, bool apply_rules);

}  // namespace loader
}  // namespace motis
