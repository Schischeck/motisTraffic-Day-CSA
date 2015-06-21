#include "motis/loader/deserializer.h"

#include <istream>
#include <fstream>

#include "motis/core/common/offset.h"
#include "motis/loader/serialize_pointer.h"
#include "motis/loader/files.h"
#include "motis/loader/binary_schedule_index.h"

namespace motis {

std::unique_ptr<char[]> read_raw_file(std::string const& file) {
  std::ifstream in(file, std::ios_base::in | std::ios_base::binary);

  auto from = in.tellg();
  in.seekg(0, std::ios::end);
  auto to = in.tellg();
  auto file_size = to - from;
  in.seekg(0, std::ios::beg);

  std::unique_ptr<char[]> ptr(new char[file_size]);

  in.read(ptr.get(), file_size);

  return ptr;
}

template <typename T>
void offsets_to_pointers(T&, char*) {}

template <typename T>
void offsets_to_pointers(pointer<T>& ptr, char* base) {
  ptr.offset_to_pointer(base);
}

template <typename T>
void offsets_to_pointers(array<T>& array, char* base) {
  array._el.offset_to_pointer(base);

  for (auto& el : array) offsets_to_pointers(el, base);
}

template <>
void offsets_to_pointers(station& station, char* base) {
  offsets_to_pointers(station.eva_nr, base);
  offsets_to_pointers(station.name, base);
}

template <>
void offsets_to_pointers(light_connection& con, char* base) {
  con._full_con.offset_to_pointer(base);
}

template <>
void offsets_to_pointers(connection& con, char* base) {
  con.con_info.offset_to_pointer(base);
}

template <>
void offsets_to_pointers(connection_info& con_info, char* base) {
  offsets_to_pointers(con_info.attributes, base);
  offsets_to_pointers(con_info.line_identifier, base);
}

template <>
void offsets_to_pointers(node& node, char* base) {
  node._station_node.offset_to_pointer(base);
  offsets_to_pointers(node._edges, base);
  offsets_to_pointers(node._incoming_edges, base);
}

template <>
void offsets_to_pointers(station_node& station_node, char* base) {
  station_node._foot_node.offset_to_pointer(base);

  offsets_to_pointers(station_node._edges, base);
  offsets_to_pointers(station_node._incoming_edges, base);

  for (auto& edge : station_node._edges) offsets_to_pointers(*edge._to, base);
}

template <>
void offsets_to_pointers(edge& edge, char* base) {
  edge._to.offset_to_pointer(base);
  edge._from.offset_to_pointer(base);

  if (edge._m._type == edge::ROUTE_EDGE)
    offsets_to_pointers(edge._m._route_edge._conns, base);
}

template <>
void offsets_to_pointers(binary_schedule_index& index, char* base) {
  offsets_to_pointers(index.stations, base);
  offsets_to_pointers(index.full_connections, base);
  offsets_to_pointers(index.connection_infos, base);
  offsets_to_pointers(index.station_nodes, base);
}

deserializer::deserializer(std::string const& prefix) : _prefix(prefix) {}

std::pair<int, std::unique_ptr<char[]>> deserializer::load_graph(
    std::vector<station_ptr>& stations,
    std::vector<station_node_ptr>& station_nodes) {
  auto schedule_buffer = read_raw_file(_prefix + SCHEDULE_FILE);
  auto index_buffer = read_raw_file(_prefix + SCHEDULE_INDEX_FILE);

  auto sched_ptr = serialize_pointer(schedule_buffer.get());
  auto index_ptr = serialize_pointer(index_buffer.get());

  auto index = index_ptr.ptr<binary_schedule_index>();
  offsets_to_pointers(*index, index_ptr.base());

  for (auto& station_offset : index->stations) {
    station& s = *sched_ptr.absolute<station>(station_offset);
    offsets_to_pointers(s, sched_ptr.base());
    stations.emplace_back(&s, deleter<station>(false));
  }

  for (auto& offset : index->full_connections)
    offsets_to_pointers(*sched_ptr.absolute<connection>(offset),
                        sched_ptr.base());

  for (auto& offset : index->connection_infos)
    offsets_to_pointers(*sched_ptr.absolute<connection_info>(offset),
                        sched_ptr.base());

  for (auto& offset : index->station_nodes) {
    station_node& n = *sched_ptr.absolute<station_node>(offset);
    offsets_to_pointers(n, sched_ptr.base());
    station_nodes.emplace_back(&n, deleter<station_node>(false));
  }

  int node_count = index->node_count;

  return {node_count, std::move(schedule_buffer)};
}

}  // namespace motis
