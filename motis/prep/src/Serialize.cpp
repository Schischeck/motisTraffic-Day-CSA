#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <type_traits>

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/loader/binary_schedule_index.h"
#include "motis/loader/bitset_manager.h"
#include "motis/loader/serialize_pointer.h"
#include "motis/loader/graph_loader.h"
#include "motis/loader/deserializer.h"
#include "motis/loader/files.h"
#include "motis/prep/services2routes.h"

#define GIGABYTE (static_cast<std::size_t>(1024 * 1024 * 1024))

namespace td {

typedef std::pair<serialize_pointer, serialize_pointer> serialize_pointer_pair;
typedef std::vector<std::pair<void const*, offset<void const*>::type>>
    offset_map;

offset<void const*>::type get_offset(offset_map const& map, void const* ptr) {
  auto comparator =
      [](std::pair<void const*, offset<void const*>::type> const& a,
         void const* b) { return a.first < b; };
  auto it = std::lower_bound(std::begin(map), std::end(map), ptr, comparator);
  if (it->first != ptr) throw std::runtime_error("not found");
  return it->second;
}

template <typename t>
std::pair<serialize_pointer, t*> simple_copy(serialize_pointer p,
                                             t const& obj) {
  auto obj_pos = p.ptr<t>();
  std::memcpy(p.ptr(), &obj, sizeof(t));
  p += sizeof(t);
  return {p, obj_pos};
}

template <typename t>
serialize_pointer_pair serialize(t const& obj, offset_map&, serialize_pointer p,
                                 serialize_pointer next_p) {
  std::memcpy(p.ptr(), &obj, sizeof(t));
  return {p + sizeof(t), next_p};
}

template <>
serialize_pointer_pair serialize(connection const& full_con,
                                 offset_map& offsets, serialize_pointer p,
                                 serialize_pointer next_p) {
  offsets.push_back(std::make_pair(&full_con, p.offset()));
  std::memcpy(p.ptr(), &full_con, sizeof(connection));
  return {p + sizeof(connection), next_p};
}

template <typename t>
serialize_pointer_pair serialize(array<t>& array, offset_map& offsets,
                                 serialize_pointer p,
                                 serialize_pointer next_p) {
  serialize_pointer start = p;
  next_p = p + (sizeof(t) * array.size());
  for (t const& el : array)
    std::tie(p, next_p) = serialize(el, offsets, p, next_p);

  array._el._offset = start.offset();
  array._self_allocated = false;
  array._allocated_size = array._used_size;

  return {next_p, next_p};
}

template <>
serialize_pointer_pair serialize(station const& s, offset_map& offsets,
                                 serialize_pointer p,
                                 serialize_pointer next_p) {
  station* copy = nullptr;
  std::tie(p, copy) = simple_copy(p, s);
  std::tie(p, std::ignore) = serialize(copy->eva_nr, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->name, offsets, p, p);
  return {p, next_p};
}

template <>
serialize_pointer_pair serialize(binary_schedule_index const& i,
                                 offset_map& offsets, serialize_pointer p,
                                 serialize_pointer next_p) {
  binary_schedule_index* copy = nullptr;
  std::tie(p, copy) = simple_copy(p, i);

  std::tie(p, std::ignore) = serialize(copy->stations, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->full_connections, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->connection_infos, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->station_nodes, offsets, p, p);

  return {p, next_p};
}

template <>
serialize_pointer_pair serialize(connection_info const& con_info,
                                 offset_map& offsets, serialize_pointer p,
                                 serialize_pointer next_p) {
  offsets.push_back(std::make_pair(&con_info, p.offset()));

  connection_info* copy = nullptr;
  std::tie(p, copy) = simple_copy(p, con_info);

  std::tie(p, std::ignore) = serialize(copy->attributes, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->line_identifier, offsets, p, p);

  return {p, next_p};
}

template <>
serialize_pointer_pair serialize(edge const& e, offset_map& offsets,
                                 serialize_pointer p,
                                 serialize_pointer next_p) {
  offsets.push_back(std::make_pair(&e, p.offset()));

  edge* copy = nullptr;
  std::tie(p, copy) = simple_copy(p, e);

  if (e._m._type == edge::ROUTE_EDGE)
    std::tie(next_p, std::ignore) =
        serialize(copy->_m._route_edge._conns, offsets, next_p, next_p);

  return {p, next_p};
}

template <>
serialize_pointer_pair serialize(node const& n, offset_map& offsets,
                                 serialize_pointer p,
                                 serialize_pointer next_p) {
  offsets.push_back(std::make_pair(&n, p.offset()));

  node* copy = nullptr;
  std::tie(p, copy) = simple_copy(p, n);

  std::tie(p, std::ignore) = serialize(copy->_edges, offsets, p, next_p);
  std::tie(p, std::ignore) =
      serialize(copy->_incoming_edges, offsets, p, next_p);

  return {p, next_p};
}

template <>
serialize_pointer_pair serialize(station_node const& n, offset_map& offsets,
                                 serialize_pointer p,
                                 serialize_pointer next_p) {
  offsets.push_back(std::make_pair(&n, p.offset()));

  station_node* copy = nullptr;
  std::tie(p, copy) = simple_copy(p, n);

  std::tie(p, next_p) = serialize(copy->_edges, offsets, p, next_p);
  std::tie(p, next_p) = serialize(copy->_incoming_edges, offsets, p, next_p);

  for (auto const& edge : make_offset_array_view(copy->_edges, p.base()))
    std::tie(p, next_p) = serialize(*edge._to, offsets, p, next_p);

  return {p, next_p};
}

template <typename t>
void pointers_to_offsets(t&, serialize_pointer const&, offset_map&) {}

template <typename t>
void pointers_to_offsets(pointer<t>& ptr, serialize_pointer const&,
                         offset_map& offsets) {
  ptr._offset = get_offset(offsets, ptr._ptr);
}

template <typename t>
void pointers_to_offsets(array<t>& array, serialize_pointer const& base,
                         offset_map& offsets) {
  for (auto& el : make_offset_array_view(array, base.base()))
    pointers_to_offsets(el, base, offsets);
}

template <>
void pointers_to_offsets(binary_schedule_index& index,
                         serialize_pointer const& base, offset_map& offsets) {
  pointers_to_offsets(index.stations, base, offsets);
  pointers_to_offsets(index.full_connections, base, offsets);
  pointers_to_offsets(index.connection_infos, base, offsets);
  pointers_to_offsets(index.station_nodes, base, offsets);
}

template <>
void pointers_to_offsets(connection_info& con_info,
                         serialize_pointer const& base, offset_map& offsets) {
  pointers_to_offsets(con_info.attributes, base, offsets);
  pointers_to_offsets(con_info.line_identifier, base, offsets);
}

template <>
void pointers_to_offsets(connection& con, serialize_pointer const&,
                         offset_map& offsets) {
  con.con_info._offset = get_offset(offsets, con.con_info._ptr);
}

template <>
void pointers_to_offsets(light_connection& con, serialize_pointer const&,
                         offset_map& offsets) {
  con._full_con._offset = get_offset(offsets, con._full_con._ptr);
}

template <>
void pointers_to_offsets(node& node, serialize_pointer const& base,
                         offset_map& offsets) {
  node._station_node._offset = get_offset(offsets, node._station_node._ptr);
  pointers_to_offsets(node._edges, base, offsets);
  pointers_to_offsets(node._incoming_edges, base, offsets);
}

template <>
void pointers_to_offsets(station_node& station_node,
                         serialize_pointer const& base, offset_map& offsets) {
  station_node._foot_node._offset =
      get_offset(offsets, station_node._foot_node._ptr);

  pointers_to_offsets(station_node._edges, base, offsets);

  for (auto& edge : make_offset_array_view(station_node._edges, base.base()))
    pointers_to_offsets(*base.absolute<node>(edge._to._offset), base, offsets);

  pointers_to_offsets(station_node._incoming_edges, base, offsets);
}

template <>
void pointers_to_offsets(edge& edge, serialize_pointer const& base,
                         offset_map& offsets) {
  edge._to._offset = get_offset(offsets, edge._to._ptr);
  edge._from._offset = get_offset(offsets, edge._from._ptr);

  if (edge._m._type == edge::ROUTE_EDGE)
    pointers_to_offsets(edge._m._route_edge._conns, base, offsets);
}

void write_file(std::string const& file_name, serialize_pointer p) {
  std::ofstream out(file_name, std::ios_base::out | std::ios_base::binary);
  out.write(p.base(), p.offset());
}

int serialize(text_schedule const& sched, std::string const& prefix) {
  binary_schedule_index index{
      sched.node_count, sched.stations.size(), sched.full_connections.size(),
      sched.connection_infos.size(), sched.station_nodes.size()};

  std::unique_ptr<char[]> memory(new char[5 * GIGABYTE]);
  serialize_pointer p(memory.get());
  offset_map offsets{{nullptr, 0}};

  // nothing should have offset 0.
  // (offsets[0] = 0 != base)
  std::fill(p.ptr(), p.ptr() + 16, 0);
  p += 16;

  // write stations.
  {
    int i = 0;
    for (auto const& station : sched.stations) {
      index.stations[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*station.get(), offsets, p, p);
    }
  }

  // write connection infos.
  {
    int i = 0;
    for (auto const& con_info : sched.connection_infos) {
      index.connection_infos[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*con_info, offsets, p, p);
    }
  }

  // write connections.
  {
    int i = 0;
    for (auto const& full_con : sched.full_connections) {
      index.full_connections[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*full_con, offsets, p, p);
    }
  }

  // write graph.
  {
    int i = 0;
    for (auto const& s_node : sched.station_nodes) {
      index.station_nodes[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*s_node, offsets, p, p);
    }
  }

  // sort offsets (-> lookup with std::lower_case is faster).
  std::sort(std::begin(offsets), std::end(offsets),
            [](std::pair<void const*, offset<void const*>::type> const& a,
               std::pair<void const*, offset<void const*>::type> const& b) {
    return a.first < b.first;
  });

  // transform pointers to offsets.
  for (auto offset : index.full_connections)
    pointers_to_offsets(*p.absolute<connection>(offset), p, offsets);

  for (auto offset : index.connection_infos)
    pointers_to_offsets(*p.absolute<connection_info>(offset), p, offsets);

  for (auto offset : index.station_nodes)
    pointers_to_offsets(*p.absolute<station_node>(offset), p, offsets);

  // write memory to file.
  write_file(prefix + SCHEDULE_FILE, p);

  // write index file.
  serialize_pointer index_p(p.base());
  std::tie(index_p, std::ignore) =
      serialize(std::move(index), offsets, index_p, index_p);
  pointers_to_offsets(*index_p.absolute<binary_schedule_index>(0), index_p,
                      offsets);
  write_file(prefix + SCHEDULE_INDEX_FILE, index_p);

  return 0;
}

}  // namespace td
