#pragma once

#include <ctime>
#include <vector>
#include <map>
#include <array>

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

struct route_section {
  route_section()
      : from_route_node(nullptr),
        to_route_node(nullptr),
        outgoing_route_edge_index(-1) {}

  route_section(node* from, node* to, int edge_idx)
      : from_route_node(from),
        to_route_node(to),
        outgoing_route_edge_index(edge_idx) {
    assert(from_route_node == nullptr || from_route_node->is_route_node());
    assert(to_route_node == nullptr || to_route_node->is_route_node());
  }

  bool is_valid() const {
    return from_route_node != nullptr && to_route_node != nullptr &&
           outgoing_route_edge_index != -1;
  }

  edge* get_route_edge() {
    if (outgoing_route_edge_index == -1) {
      return nullptr;
    }

    assert(outgoing_route_edge_index >= 0);
    assert(static_cast<unsigned>(outgoing_route_edge_index) <
           from_route_node->_edges.size());
    assert(from_route_node->_edges[outgoing_route_edge_index].type() ==
           edge::ROUTE_EDGE);
    return &from_route_node->_edges[outgoing_route_edge_index];
  }

  node* from_route_node;
  node* to_route_node;
  int outgoing_route_edge_index;
};

struct participant {
  participant() : service(nullptr), section_idx(0) {}

  participant(Service const* service, int section_idx)
      : service(service), section_idx(section_idx) {}

  friend bool operator<(participant const& lhs, participant const& rhs) {
    return lhs.service > rhs.service;
  }

  friend bool operator>(participant const& lhs, participant const& rhs) {
    return lhs.service < rhs.service;
  }

  friend bool operator==(participant const& lhs, participant const& rhs) {
    return lhs.service == rhs.service;
  }

  Service const* service;
  int section_idx;
};

typedef std::vector<route_section> route;
typedef std::vector<std::vector<light_connection>> route_lcs;

struct graph_builder {
  graph_builder(schedule& sched, Interval const* schedule_interval, time_t from,
                time_t to, bool apply_rules, bool adjust_footpaths);

  void add_stations(
      flatbuffers::Vector<flatbuffers::Offset<Station>> const* stations);

  void link_meta_stations(
      flatbuffers::Vector<flatbuffers::Offset<MetaStation>> const*
          meta_stations);

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

  connection_info* get_or_create_connection_info(Section const* section,
                                                 int dep_day_index,
                                                 connection_info* merged_with);

  connection_info* get_or_create_connection_info(
      std::array<participant, 16> const& services, int dep_day_index);

  light_connection section_to_connection(
      std::array<participant, 16> const& services, int day, time prev_arr,
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

  std::unique_ptr<route> create_route(Route const* r, route_lcs const& lcons,
                                      unsigned route_index);

  node* build_route_node(int route_index, Station const* station,
                         bool in_allowed, bool out_allowed);

  route_section add_route_section(
      int route_index, std::vector<light_connection> const& connections,
      Station const* from_stop, bool from_in_allowed, bool from_out_allowed,
      Station const* to_stop, bool to_in_allowed, bool to_out_allowed,
      route_section prev_section, route_section next_section);

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
  bool adjust_footpaths_;

  connection_info con_info_;
  connection con_;
};

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool check_unique, bool apply_rules,
                         bool adjust_footpaths);

}  // namespace loader
}  // namespace motis
