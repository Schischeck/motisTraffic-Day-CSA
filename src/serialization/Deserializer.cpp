#include "serialization/Deserializer.h"

#include <istream>
#include <fstream>

#include "serialization/SerializePointer.h"
#include "serialization/Offset.h"
#include "serialization/Files.h"
#include "serialization/Print.h"

namespace td {

std::unique_ptr<char[]> readRawFile(std::string const& file)
{
  std::ifstream in(file, std::ios_base::in | std::ios_base::binary);

  auto from = in.tellg();
  in.seekg(0, std::ios::end);
  auto to = in.tellg();
  auto fileSize = to - from;
  in.seekg(0, std::ios::beg);

  std::unique_ptr<char[]> ptr(new char[fileSize]);

  in.read(ptr.get(), fileSize);

  return ptr;
}

template<typename T>
void offsetsToPointers(T&,  char*)
{}

template<typename T>
void offsetsToPointers(Array<T>& array, char* base)
{
  array._el.offsetToPointer(base);

  for (auto& el : array)
    offsetsToPointers(el, base);
}

template<>
void offsetsToPointers(Station& station, char* base)
{
  offsetsToPointers(station.evaNr, base);
  offsetsToPointers(station.name, base);
}

template<>
void offsetsToPointers(LightConnection& con, char* base)
{
  con._fullCon.offsetToPointer(base);
  con._next.offsetToPointer(base);
}

template<>
void offsetsToPointers(Connection& con, char* base)
{ con.conInfo.offsetToPointer(base); }

template<>
void offsetsToPointers(ConnectionInfo& conInfo, char* base)
{
  offsetsToPointers(conInfo.attributes, base);
  offsetsToPointers(conInfo.lineIdentifier, base);
}

template<>
void offsetsToPointers(Node& node, char* base)
{
  node._stationNode.offsetToPointer(base);
  offsetsToPointers(node._edges, base);
}

template<>
void offsetsToPointers(StationNode& stationNode, char* base)
{
  stationNode._footNode.offsetToPointer(base);

  offsetsToPointers(stationNode._edges, base);

  for (auto& edge : stationNode._edges)
    offsetsToPointers(*edge._to, base);
}

template<>
void offsetsToPointers(Edge& edge, char* base)
{
  edge._to.offsetToPointer(base);

  if (edge._m._type == Edge::ROUTE_EDGE)
    offsetsToPointers(edge._m._routeEdge._conns, base);
}

template<>
void offsetsToPointers(Deserializer::Index& index, char* base)
{
  offsetsToPointers(index.stations, base);
  offsetsToPointers(index.fullConnections, base);
  offsetsToPointers(index.connectionInfos, base);
  offsetsToPointers(index.stationNodes, base);
}

Deserializer::Deserializer(std::string const& prefix) : _prefix(prefix) {}

std::pair<int, std::unique_ptr<char[]>>
Deserializer::loadGraph(
    std::vector<StationPtr>& stations,
    std::vector<StationNodePtr>& stationNodes)
{
  auto scheduleBuffer = readRawFile(_prefix + SCHEDULE_FILE);
  auto indexBuffer = readRawFile(_prefix + SCHEDULE_INDEX_FILE);

  auto schedPtr = SerializePointer(scheduleBuffer.get());
  auto indexPtr = SerializePointer(indexBuffer.get());

  Index* index = indexPtr.ptr<Index>();
  offsetsToPointers(*index, indexPtr.base());

  for (auto& offset : index->stations)
  {
    Station& station = *schedPtr.absolute<Station>(offset);
    offsetsToPointers(station, schedPtr.base());
    stations.emplace_back(&station, Deleter<Station>(false));
  }

  for (auto& offset : index->fullConnections)
    offsetsToPointers(*schedPtr.absolute<Connection>(offset), schedPtr.base());

  for (auto& offset : index->connectionInfos)
    offsetsToPointers(*schedPtr.absolute<ConnectionInfo>(offset), schedPtr.base());

  for (auto& offset : index->stationNodes)
  {
    StationNode& stationNode = *schedPtr.absolute<StationNode>(offset);
    offsetsToPointers(stationNode, schedPtr.base());
    stationNodes.emplace_back(&stationNode, Deleter<StationNode>(false));
  }

  int nodeCount = index->nodeCount;

  return { nodeCount, std::move(scheduleBuffer) };
}

}  // namespace td
