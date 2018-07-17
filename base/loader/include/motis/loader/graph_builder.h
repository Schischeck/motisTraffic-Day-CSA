#pragma once

#include "flatbuffers/flatbuffers.h"

#include "motis/core/common/hash_helper.h"
#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/timezone.h"

#include "motis/loader/bitfield.h"
#include "motis/loader/util.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace loader {

struct lcon_times {
  lcon_times() = default;
  lcon_times(uint16_t dep, uint16_t arr) : d_time_(dep), a_time_(arr) {}
  friend bool operator<(lcon_times const& a, lcon_times const& b) {
    return std::tie(a.d_time_, a.a_time_) < std::tie(b.d_time_, b.a_time_);
  }
  friend bool operator==(lcon_times const& a, lcon_times const& b) {
    return std::tie(a.d_time_, a.a_time_) == std::tie(b.d_time_, b.a_time_);
  }
  uint16_t d_time_, a_time_;
};

struct route_section {
  route_section()
      : from_route_node_(nullptr),
        to_route_node_(nullptr),
        outgoing_route_edge_index_(-1) {}

  route_section(node* from, node* to, int edge_idx)
      : from_route_node_(from),
        to_route_node_(to),
        outgoing_route_edge_index_(edge_idx) {
    assert(from_route_node_ == nullptr || from_route_node_->is_route_node());
    assert(to_route_node_ == nullptr || to_route_node_->is_route_node());
  }

  bool is_valid() const {
    return from_route_node_ != nullptr && to_route_node_ != nullptr &&
           outgoing_route_edge_index_ != -1;
  }

  edge* get_route_edge() {
    if (outgoing_route_edge_index_ == -1) {
      return nullptr;
    }

    assert(outgoing_route_edge_index_ >= 0);
    assert(static_cast<unsigned>(outgoing_route_edge_index_) <
           from_route_node_->edges_.size());
    assert(from_route_node_->edges_[outgoing_route_edge_index_].type() ==
           edge::ROUTE_EDGE);
    return &from_route_node_->edges_[outgoing_route_edge_index_];
  }

  node* from_route_node_;
  node* to_route_node_;
  int outgoing_route_edge_index_;
};

typedef std::vector<route_section> route;

struct route_t {

  route_t() = default;
  route_t(std::vector<light_connection> const& new_lcons,
          std::vector<time> const& times) {
    lcons_.emplace_back(new_lcons);
    times_.emplace_back(times);
  }

  bool add_service(std::vector<light_connection> const& new_lcons,
                   std::vector<time> const& new_times) {
    verify(std::all_of(begin(lcons_), end(lcons_),
                       [&new_lcons](auto const& i) {
                         return new_lcons.size() == i.size();
                       }),
           "number of sections not matching");

    verify(new_lcons.size() * 2 == new_times.size(),
           "number of times and lcons not matching");

    auto const& insert_it = std::lower_bound(
        begin(times_), end(times_), new_times,
        [](std::vector<time> const& lhs, std::vector<time> const& rhs) {
          return lhs.front() < rhs.front();
        });
    auto const insert_idx = std::distance(begin(times_), insert_it);

    // check full time array!
    // Example: insert [3,9,14,18] into [[4,10,12,16]]
    // check 3 < 4 -> ok
    // check 9 < 10 -> ok
    // check 14 < 12 -> fail, new service overtakes the existing
    for (unsigned i = 0; i < new_times.size(); ++i) {
      auto middle_time = new_times[i].mam();
      bool before_pred = false;
      bool after_succ = false;
      if (times_.size() != 0) {
        if (insert_idx != 0) {
          auto before_time =
              times_[insert_idx - 1][i].mam();
          before_pred = middle_time <= before_time;
        }
        if (static_cast<int>(insert_idx) < static_cast<int>(times_.size())) {
          auto after_time = times_[insert_idx][i].mam();
          after_succ = middle_time >= after_time;
        }
      }
      if (before_pred || after_succ) {
        return false;
      }
    }

    // new_s is safe to add at idx
    times_.insert(std::next(begin(times_), insert_idx), new_times);
    lcons_.insert(std::next(begin(lcons_), insert_idx), new_lcons);
    verify(
        std::is_sorted(times_.begin(), times_.end(),
                       [](std::vector<time> const& a,
                          std::vector<time> const& b) { return a[0] < b[0]; }),
        "route services not sorted");

    return true;
  }

  bool empty() const { return times_.empty(); }

  std::vector<light_connection> const& operator[](std::size_t idx) const {
    return lcons_[idx];
  }

  std::vector<std::vector<time>> times_;
  std::vector<std::vector<light_connection>> lcons_;
};

template <typename T, typename... Args>
inline std::size_t push_mem(std::vector<std::unique_ptr<T>>& elements,
                            Args... args) {
  auto idx = elements.size();
  elements.emplace_back(new T(args...));
  return idx;
}

struct graph_builder {
  graph_builder(schedule& sched, Interval const* schedule_interval, time_t from,
                time_t to, bool apply_rules, bool adjust_footpaths);

  void add_dummy_node(std::string const& name);

  void add_stations(
      flatbuffers64::Vector<flatbuffers64::Offset<Station>> const* stations);

  void link_meta_stations(
      flatbuffers64::Vector<flatbuffers64::Offset<MetaStation>> const*
          meta_stations);

  void add_services(
      flatbuffers64::Vector<flatbuffers64::Offset<Service>> const* services);

  void add_route_services(std::vector<Service const*> const& services);

  std::vector<std::pair<std::vector<time>, std::unordered_set<unsigned>>>
  service_times_to_utc(bitfield const& traffic_days, int start_idx, int end_idx,
                       Service const* s);

  merged_trips_idx create_merged_trips(Service const* s, int day_idx);

  trip* register_service(const Service* s, int day_idx);

  station_node* get_station_node(Station const* station) const;

  full_trip_id get_full_trip_id(Service const* s, int day,
                                int section_idx = 0) const;

  light_connection section_to_connection(
      unsigned section_idx, Service const* service,
      std::vector<time> relative_utc,
      std::unordered_set<unsigned> srv_traffic_days,
      merged_trips_idx trips_idx);

  void count_events(const Service* const& service, int section_idx);

  void add_to_routes(std::vector<route_t>& alt_routes,
                     std::vector<time> const& times,
                     std::vector<light_connection> const& lcons);

  std::unique_ptr<route> create_route(Route const* r, route_t const& lcons,
                                      unsigned route_index);

  route_section add_route_section(
      int route_index, std::vector<light_connection> const& connections,
      Station const* from_stop, bool from_in_allowed, bool from_out_allowed,
      Station const* to_stop, bool to_in_allowed, bool to_out_allowed,
      node* from_route_node, node* to_route_node);

  void index_first_route_node(route const& r);

  const std::string* get_or_create_direction(const Direction* dir);

  const provider* get_or_create_provider(const Provider* p);

  int get_or_create_category_index(const Category* c);

  void write_trip_info(route&);

  void add_footpaths(
      flatbuffers64::Vector<flatbuffers64::Offset<Footpath>> const* footpaths);

  void connect_reverse();

  void sort_connections();
  void sort_trips();
  void dedup_bitfields();

  timezone const* get_or_create_timezone(Timezone const* input_timez);

  size_t get_or_create_bitfield(
      flatbuffers64::String const* serialized_bitfield);

  size_t get_or_create_bitfield(bitfield const&);

  connection_info* get_or_create_connection_info(Section const* section,
                                                 int dep_day_index);

  connection_info* get_or_create_connection_info(Service const* service,
                                                 unsigned section_idx,
                                                 int dep_day_index);

  connection* get_full_connection(
      const Service* services, const station& from, const station& to,
      const flatbuffers64::Vector<flatbuffers64::Offset<Track>>* dep_platf,
      const flatbuffers64::Vector<flatbuffers64::Offset<Track>>* arr_platf,
      unsigned section_idx);

  connection* get_full_connection(unsigned int section_idx,
                                  const Service* service);

  schedule& sched_;
  unsigned first_day_, last_day_;
  unsigned from_day_, to_day_;
  bool apply_rules_;
  bool adjust_footpaths_;

  std::map<Category const*, int> categories_;
  std::map<std::string, int> tracks_;
  std::map<flatbuffers64::String const*, std::string const*> directions_;
  std::map<Provider const*, provider const*> providers_;
  hash_map<Station const*, station_node*> stations_;
  std::map<Timezone const*, timezone const*> timezones_;
  hash_set<connection_info*,
           deep_ptr_hash<connection_info::hash, connection_info>,
           deep_ptr_eq<connection_info>>
      con_infos_;
  hash_set<connection*, deep_ptr_hash<connection::hash, connection>,
           deep_ptr_eq<connection>>
      connections_;

  unsigned next_node_id_;
  unsigned next_route_index_;

  connection_info con_info_;
  connection con_;

  unsigned lcon_count_ = 0;
};

}  // namespace loader
}  // namespace motis
