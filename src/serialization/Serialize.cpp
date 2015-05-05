#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <type_traits>

#include "GraphLoader.h"
#include "BitsetManager.h"
#include "Nodes.h"
#include "Station.h"
#include "Connection.h"

#include "serialization/Print.h"
#include "serialization/SerializePointer.h"
#include "serialization/Schedule.h"
#include "serialization/Deserializer.h"
#include "serialization/Services2Routes.h"

#define GIGABYTE (static_cast<std::size_t>(1024*1024*1024))

namespace td {

typedef std::pair<SerializePointer, SerializePointer> SerializePointerPair;
typedef std::vector<std::pair<void const*, Offset<void const*>::type>> OffsetMap;

typename Offset<void const*>::type getOffset(
    OffsetMap const& map,
    void const* ptr)
{
  auto comparator =
      [](std::pair<void const *, Offset<void const *>::type> const& a,
         void const* b) { return a.first < b; };
  auto it = std::lower_bound(std::begin(map), std::end(map), ptr, comparator);
  if (it->first != ptr)
    throw std::runtime_error("not found");
  return it->second;
}

template<typename T>
std::pair<SerializePointer, T*> simpleCopy(SerializePointer p, T const& obj)
{
  auto objPos = p.ptr<T>();
  std::memcpy(p.ptr(), &obj, sizeof(T));
  p += sizeof(T);
  return { p, objPos };
}

template <typename T>
SerializePointerPair serialize(
    T const& obj,
    OffsetMap&,
    SerializePointer p, SerializePointer nextP)
{
  std::memcpy(p.ptr(), &obj, sizeof(T));
  return { p + sizeof(T), nextP };
}

template <>
SerializePointerPair serialize(
    LightConnection const& lightCon,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  offsets.push_back(std::make_pair(&lightCon, p.offset()));
  std::memcpy(p.ptr(), &lightCon, sizeof(LightConnection));
  return { p + sizeof(LightConnection), nextP };
}

template <>
SerializePointerPair serialize(
    Connection const& fullCon,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  offsets.push_back(std::make_pair(&fullCon, p.offset()));
  std::memcpy(p.ptr(), &fullCon, sizeof(Connection));
  return { p + sizeof(Connection), nextP };
}

template <typename T>
SerializePointerPair serialize(
    Array<T>& array,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  SerializePointer start = p;
  nextP = p + (sizeof(T) * array.size());
  for (T const& el : array)
    std::tie(p, nextP) = serialize(el, offsets, p, nextP);

  array._el._offset = start.offset();
  array._selfAllocated = false;
  array._allocatedSize = array._usedSize;

  return { nextP, nextP };
}

template <>
SerializePointerPair serialize(
    Station const& station,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  Station* copy = nullptr;
  std::tie(p, copy) = simpleCopy(p, station);
  std::tie(p, std::ignore) = serialize(copy->evaNr, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->name, offsets, p, p);
  return { p, nextP };
}

template <>
SerializePointerPair serialize(
    Deserializer::Index const& index,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  Deserializer::Index* copy = nullptr;
  std::tie(p, copy) = simpleCopy(p, index);

  std::tie(p, std::ignore) = serialize(copy->stations, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->fullConnections, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->connectionInfos, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->stationNodes, offsets, p, p);

  return { p, nextP };
}

template <>
SerializePointerPair serialize(
    ConnectionInfo const& conInfo,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  offsets.push_back(std::make_pair(&conInfo, p.offset()));

  ConnectionInfo* copy = nullptr;
  std::tie(p, copy) = simpleCopy(p, conInfo);

  std::tie(p, std::ignore) = serialize(copy->attributes, offsets, p, p);
  std::tie(p, std::ignore) = serialize(copy->lineIdentifier, offsets, p, p);

  return { p, nextP };
}

template <>
SerializePointerPair serialize(
    Edge const& edge,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  Edge* copy = nullptr;
  std::tie(p, copy) = simpleCopy(p, edge);

  if (edge._m._type == Edge::ROUTE_EDGE)
    std::tie(nextP, std::ignore) =
        serialize(copy->_m._routeEdge._conns, offsets, nextP, nextP);

  return { p, nextP };
}

template <>
SerializePointerPair serialize(
    Node const& node,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  offsets.push_back(std::make_pair(&node, p.offset()));

  Node* copy = nullptr;
  std::tie(p, copy) = simpleCopy(p, node);

  std::tie(p, std::ignore) = serialize(copy->_edges, offsets, p, nextP);

  return { p, nextP };
}

template <>
SerializePointerPair serialize(
    StationNode const& stationNode,
    OffsetMap& offsets,
    SerializePointer p, SerializePointer nextP)
{
  offsets.push_back(std::make_pair(&stationNode, p.offset()));

  StationNode* copy = nullptr;
  std::tie(p, copy) = simpleCopy(p, stationNode);

  std::tie(p, nextP) = serialize(copy->_edges, offsets, p, nextP);

  for (auto const& edge : makeOffsetArrayView(copy->_edges, p.base()))
    std::tie(p, nextP) = serialize(*edge._to, offsets, p, nextP);

  return { p, nextP };
}

template<typename T>
void pointersToOffsets(T&, SerializePointer const&, OffsetMap&) {}

template<typename T>
void pointersToOffsets(
    Array<T>& array, SerializePointer const& base, OffsetMap& offsets)
{
  for (auto& el : makeOffsetArrayView(array, base.base()))
    pointersToOffsets(el, base, offsets);
}

template <>
void pointersToOffsets(
    Deserializer::Index& index,
    SerializePointer const& base,
    OffsetMap& offsets)
{
  pointersToOffsets(index.stations, base, offsets);
  pointersToOffsets(index.fullConnections, base, offsets);
  pointersToOffsets(index.connectionInfos, base, offsets);
  pointersToOffsets(index.stationNodes, base, offsets);
}

template<>
void pointersToOffsets(
    ConnectionInfo& conInfo,
    SerializePointer const& base,
    OffsetMap& offsets)
{
  pointersToOffsets(conInfo.attributes, base, offsets);
  pointersToOffsets(conInfo.lineIdentifier, base, offsets);
}

template<>
void pointersToOffsets(
    Connection& con,
    SerializePointer const&,
    OffsetMap& offsets)
{ con.conInfo._offset = getOffset(offsets, con.conInfo._ptr); }

template<>
void pointersToOffsets(
    LightConnection& con,
    SerializePointer const&,
    OffsetMap& offsets)
{
  con._next._offset = getOffset(offsets, con._next._ptr);
  con._fullCon._offset = getOffset(offsets, con._fullCon._ptr);
}

template<>
void pointersToOffsets(
    Node& node,
    SerializePointer const& base,
    OffsetMap& offsets)
{
  node._stationNode._offset = getOffset(offsets, node._stationNode._ptr);
  pointersToOffsets(node._edges, base, offsets);
}

template<>
void pointersToOffsets(
    StationNode& stationNode,
    SerializePointer const& base,
    OffsetMap& offsets)
{
  stationNode._footNode._offset = getOffset(offsets, stationNode._footNode._ptr);

  pointersToOffsets(stationNode._edges, base, offsets);

  for (auto& edge : makeOffsetArrayView(stationNode._edges, base.base()))
    pointersToOffsets(*base.absolute<Node>(edge._to._offset), base, offsets);
}

template<>
void pointersToOffsets(
    Edge& edge,
    SerializePointer const& base,
    OffsetMap& offsets)
{
  edge._to._offset = getOffset(offsets, edge._to._ptr);

  if (edge._m._type == Edge::ROUTE_EDGE)
    pointersToOffsets(edge._m._routeEdge._conns, base, offsets);
}

void writeFile(std::string const& fileName, SerializePointer p)
{
  std::ofstream out(fileName, std::ios_base::out | std::ios_base::binary);
  out.write(p.base(), p.offset());
}

int serialize(Schedule const& sched, std::string const& prefix) {
  Deserializer::Index index {
      sched.nodeCount,
      sched.stations.size(),
      sched.fullConnections.size(),
      sched.connectionInfos.size(),
      sched.stationNodes.size()
  };

  std::unique_ptr<char[]>  memory(new char[5 * GIGABYTE]);
  SerializePointer p(memory.get());
  OffsetMap offsets { { nullptr, 0 } };

  // Nothing should have offset 0.
  // (offsets[0] = 0 != base)
  std::fill(p.ptr(), p.ptr() + 16, 0);
  p += 16;

  // Write stations.
  {
    int i = 0;
    for (auto const& station : sched.stations)
    {
      index.stations[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*station.get(), offsets, p, p);
    }
  }

  // Write connection infos.
  {
    int i = 0;
    for (auto const& conInfo : sched.connectionInfos)
    {
      index.connectionInfos[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*conInfo, offsets, p, p);
    }
  }

  // Write connections.
  {
    int i = 0;
    for (auto const& fullCon : sched.fullConnections)
    {
      index.fullConnections[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*fullCon, offsets, p, p);
    }
  }

  // Write graph.
  {
    int i = 0;
    for (auto const& sNode : sched.stationNodes)
    {
      index.stationNodes[i++] = p.offset();
      std::tie(p, std::ignore) = serialize(*sNode, offsets, p, p);
    }
  }

  // Sort offsets (-> lookup with std::lower_case is faster).
  std::sort(std::begin(offsets), std::end(offsets),
            [](std::pair<void const*, Offset<void const*>::type> const& a,
               std::pair<void const*, Offset<void const*>::type> const& b)
            { return a.first < b.first; });

  // Transform pointers to offsets.
  for (auto offset : index.fullConnections)
    pointersToOffsets(*p.absolute<Connection>(offset), p, offsets);

  for (auto offset : index.connectionInfos)
    pointersToOffsets(*p.absolute<ConnectionInfo>(offset), p, offsets);

  for (auto offset : index.stationNodes)
    pointersToOffsets(*p.absolute<StationNode>(offset), p, offsets);

  // Write memory to file.
  writeFile(prefix + SCHEDULE_FILE, p);

  // Write index file.
  SerializePointer indexP(p.base());
  std::tie(indexP, std::ignore) = serialize(std::move(index), offsets,
                                            indexP, indexP);
  pointersToOffsets(*indexP.absolute<Deserializer::Index>(0), indexP, offsets);
  writeFile(prefix + SCHEDULE_INDEX_FILE, indexP);

  return 0;
}

}  // namespace td
